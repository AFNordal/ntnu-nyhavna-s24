/*
HVA GJENSTÅR?
-------------
Nå fungerer det kanskje. Bør testes over lang tid.

Får av og til FR_DISK_ERR fra f_write, kun når man 
skriver til IMU og DMA-ene til F9P-ene kjører.

Det virker som at det er forskjell på Release og
Debug.

Lurer på om feilen har vært fordi dma-bufrene ikke
var erklert volatile. Det er de fortsatt ikke (Fordi 
sd_write ikke tar imot volatile void*), men har satt
inn en __sync_synchronize før hver sd_write.

Dersom feilene vedvarer, se etter nytt SD-lib

Ellers er egentlig programmet ferdig.

HUSK:
 - FATAL() i sd_write, som halt-er også i Release-modus
 - Står nå i Release-modus

*/

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "sys_utils.h"
#include "bmi270.h"
#include "sd_utils.h"
#include "battery.h"
#include "f9p.h"

#define IMU_BUF_SIZE 512
#define F9P_INTERRUPT_INTERVAL 2 // seconds
#define F9P_INTERRUPT_PIN 22
#define F9P_RX0_PIN 1
#define F9P_RX1_PIN 9
#define BMI_DRDY_PIN 6

queue_t IMU_queue;

volatile bool disconnected = false;
volatile bool IMU_drdy = false;
volatile uint32_t IMU_ticker = 0;
volatile uint32_t stamp_offset = 0;

typedef struct
{
    bmi_data_t data;
    uint32_t sample_idx;
    uint8_t stamped;
    uint32_t _dummy0[3]; // To make the struct 32 bytes long
} IMU_sample_t;
int a = sizeof(bmi_data_t);
void core1_entry(void);

void __time_critical_func(IMU_drdy_handler)(uint gpio, uint32_t event_mask)
{
    if (IMU_ticker++ >= stamp_offset + BMI_ODR_HZ * F9P_INTERRUPT_INTERVAL)
    {
        f9p_send_interrupt();
        IMU_ticker = 0;
        if (++stamp_offset >= 64)
            stamp_offset = 0;
    }
    IMU_drdy = true;
}

void __time_critical_func(adc_handler)(uint16_t level)
{
    if (level < 1600)
        disconnected = true;
}

int main()
{
    stdio_init_all();
    set_sys_clock_133mhz();
    busy_wait_cycles_ms(1000);

    queue_init(&IMU_queue, sizeof(IMU_sample_t) * IMU_BUF_SIZE, 1);
    multicore_launch_core1(core1_entry);
    // INFO("Launched core 1\n");

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

    uint16_t write_count = 0;
    IMU_sample_t *IMUbuffer = new IMU_sample_t[IMU_BUF_SIZE];
    while (!disconnected)
    {
        uint8_t *buf;
        if (f9p_chan0_drdy(&buf))
        {
            sd_write(&F9Pfile0, buf, F9P_BUF_SIZE);
        }
        if (f9p_chan1_drdy(&buf))
        {
            sd_write(&F9Pfile1, buf, F9P_BUF_SIZE);
        }

        if (!queue_try_remove(&IMU_queue, IMUbuffer))
            continue;
        sd_write(&IMUfile, IMUbuffer, sizeof(IMU_sample_t) * IMU_BUF_SIZE);
        // Flush file periodically
        if ((++write_count) % 16 == 0)
        {
            sd_sync(&IMUfile);
            // INFO("Flushed file\n");
        }
    }
    sd_close(&IMUfile);
    sd_close(&F9Pfile0);
    sd_close(&F9Pfile1);
    sd_unmount();
    INFO("SD card unmounted\n");

    blink();
    for (;;)
        ;
    
}

void core1_entry(void)
{

    bmi_init();
    bmi_set_drdy_pin(BMI_DRDY_PIN, IMU_drdy_handler);
    // INFO("BMI270 initialized\n");

    uint32_t IMU_sample_idx = 0;
    IMU_sample_t *IMUbuffer = new IMU_sample_t[IMU_BUF_SIZE];

    // Flush first readings
    for (uint16_t i = 0; i < 128; i++)
    {
        while (!IMU_drdy)
            ;
        IMU_drdy = false;
        bmi_read_FIFO(nullptr, bmi_get_FIFO_length());
    }
    while (true)
    {
        if (!IMU_drdy)
            // No data ready
            continue;
        IMU_drdy = false;
        bmi_data_t IMUdata;
        if (bmi_read_sensors(&IMUdata) == 1)
            // False drdy; This happens some times
            continue;
        bool stamped = (IMU_ticker == 0);
        IMUbuffer[IMU_sample_idx % IMU_BUF_SIZE] = (IMU_sample_t){IMUdata,
                                                                  IMU_sample_idx++,
                                                                  stamped};
        if (IMU_sample_idx % IMU_BUF_SIZE == 0)
        {
            if (!queue_try_add(&IMU_queue, IMUbuffer))
                ERROR("Queue full\n");
        }
    }
}
