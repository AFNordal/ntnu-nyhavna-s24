#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "sys_utils.h"
#include "bmi270.h"
#include "sd_utils.h"
#include "battery.h"
#include "f9p.h"

// No. of 32 bytes long structs to be buffered before writing to sd
#define IMU_BUF_SIZE 1536
#define F9P_INTERRUPT_INTERVAL_S 2
#define F9P_INTERRUPT_PIN 22
#define F9P_RX0_PIN 1
#define F9P_RX1_PIN 9
#define BMI_DRDY_PIN 6

// Used to communicate to core 0 that core 1 is alive
queue_t heartbeat_queue;

// Whether main power is disconnected
volatile bool pwr_disconnected = false;
// Whether IMU has new data
volatile bool IMU_drdy = false;
// Whether the IMU is ready to take real readings
volatile bool IMU_armed = false;
// Whether the most recent IMU sample coincides with a pulse to the F9P's
volatile bool IMU_sample_stamped = false;
volatile uint32_t IMU_sample_idx = 0;
queue_t IMU_queue;

typedef struct
{
    bmi_data_t data;
    uint32_t sample_idx;
    uint8_t stamped;
    uint32_t _dummy0[3]; // To make the struct 32 bytes long
} IMU_sample_t;

void core1_entry(void);

void IMU_drdy_handler(uint gpio, uint32_t event_mask)
{
    static uint8_t stamp_offset = 32;
    static uint32_t prev_stamped = 0;
    if (IMU_armed && (IMU_sample_idx - prev_stamped == stamp_offset + BMI_ODR_HZ * F9P_INTERRUPT_INTERVAL_S))
    {
        prev_stamped = IMU_sample_idx;
        IMU_sample_stamped = true;
        f9p_send_interrupt();
        if (--stamp_offset == 0)
            stamp_offset = 64;
    }
    else
    {
        IMU_sample_stamped = false;
    }
    IMU_drdy = true;
}

// Checks vbus voltage
void adc_handler(uint16_t level)
{
    if (level < 1600)
        pwr_disconnected = true;
}

int main()
{
    stdio_init_all();
    set_sys_clock_133mhz();
    led_init();
    busy_wait_cycles_ms(4000);

    queue_init(&heartbeat_queue, sizeof(uint8_t), 1);

    FATFS fs;
    sd_mount(&fs);

    // Read 'count' from .count file; Increment count and use
    // result in file names to avoid overwriting.
    FIL count_file;
    sd_open(&count_file, ".count", FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
    char count_buf[32];
    uint32_t count;
    if (sd_readnum(&count_file, &count) != PICO_OK)
        count = 0;
    else
        count++;
    sd_rewind(&count_file);
    sd_writenum(&count_file, count);
    sd_close(&count_file);

    // Create IMU file
    FIL IMUfile;
    char filename[32];
    sprintf(filename, "IMU-%d.bin", count);
    sd_open(&IMUfile, filename, FA_CREATE_ALWAYS | FA_WRITE);

    // Create F9P files
    FIL F9Pfile0, F9Pfile1;
    char filename0[32], filename1[32];
    sprintf(filename0, "F9P0-%d.bin", count);
    sprintf(filename1, "F9P1-%d.bin", count);
    sd_open(&F9Pfile0, filename0, FA_CREATE_ALWAYS | FA_WRITE);
    sd_open(&F9Pfile1, filename1, FA_CREATE_ALWAYS | FA_WRITE);

    INFO("SD file system initialized\n");

    alarm_pool_t *core0_alarms = alarm_pool_get_default();
    // Poll capacitor voltage every 5ms
    battery_init(5, adc_handler, core0_alarms);
    INFO("Battery management initialized\n");

    // Timer used to pulse interrupt pin
    f9p_init(F9P_RX0_PIN, F9P_RX1_PIN, F9P_INTERRUPT_PIN, core0_alarms);
    INFO("F9P's initialized\n");

    queue_init(&IMU_queue, sizeof(IMU_sample_t) * IMU_BUF_SIZE, 1);
    multicore_launch_core1(core1_entry);
    INFO("Launched core 1\n");

    uint16_t write_count = 0;
    IMU_sample_t *IMUbuffer = new IMU_sample_t[IMU_BUF_SIZE];
    uint8_t *F9Pbuffer = new uint8_t[F9P_BUF_SIZE];
    while (!pwr_disconnected)
    {
        heartbeat_blink(&heartbeat_queue);

        uint32_t datarate;
        if (f9p_chan0_drdy(&F9Pbuffer))
        {
            sd_write(&F9Pfile0, F9Pbuffer, F9P_BUF_SIZE);
            sd_sync(&F9Pfile0);
            INFO("Wrote F9P0 file\n");
        }
        if (f9p_chan1_drdy(&F9Pbuffer))
        {
            sd_write(&F9Pfile1, F9Pbuffer, F9P_BUF_SIZE);
            sd_sync(&F9Pfile1);
            INFO("Wrote F9P1 file\n");
        }

        if (!queue_try_remove(&IMU_queue, IMUbuffer))
            continue;
        sd_write(&IMUfile, IMUbuffer, sizeof(IMU_sample_t) * IMU_BUF_SIZE);
        // INFO("Wrote IMU file\n");
        if ((++write_count) % 16 == 0)
        {
            sd_sync(&IMUfile);
            INFO("Flushed IMU file\n");
        }
    }
    sd_close(&IMUfile);
    sd_close(&F9Pfile0);
    sd_close(&F9Pfile1);
    sd_unmount();
    INFO("SD card unmounted\n");

    info_blink();
    for (;;)
        ;
}

void core1_entry(void)
{
    // Dummy variable
    uint8_t heartbeat_const = 1;

    bmi_init();
    bmi_set_drdy_pin(BMI_DRDY_PIN, IMU_drdy_handler);
    INFO("BMI270 initialized\n");

    IMU_sample_t *IMUbuffer = new IMU_sample_t[IMU_BUF_SIZE];

    // Flush first readings
    for (uint16_t i = 0; i < 128; i++)
    {
        while (!IMU_drdy)
            ;
        IMU_drdy = false;
        bmi_read_FIFO(nullptr, bmi_get_FIFO_length());
    }
    IMU_armed = true;
    while (true)
    {
        queue_try_add(&heartbeat_queue, &heartbeat_const);

        if (!IMU_drdy)
            // No data ready
            continue;
        IMU_drdy = false;
        bmi_data_t IMUdata;
        if (bmi_read_sensors(&IMUdata) == 1)
        {
            // False drdy; This happens some times
            INFO("False drdy\n");
            continue;
        }
        IMUbuffer[IMU_sample_idx % IMU_BUF_SIZE] = (IMU_sample_t){IMUdata,
                                                                  IMU_sample_idx,
                                                                  IMU_sample_stamped};
        IMU_sample_idx++;
        if (IMU_sample_idx % IMU_BUF_SIZE == 0)
        {
            if (!queue_try_add(&IMU_queue, IMUbuffer))
                ERROR("Queue full\n");
        }
    }
}
