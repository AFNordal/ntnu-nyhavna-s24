#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "sys_utils.h"
#include "bmi270.h"
#include "sd_utils.h"
#include "battery.h"

#define IMU_BUF_SIZE 1024

queue_t IMU_queue;

volatile bool disconnected = false;

typedef struct
{
    bmi_data_t data;
    uint32_t sample_idx;
    uint8_t stamped;
} IMU_sample_t;

void process_bmi_fifo(void);
void core1_entry(void);

int main()
{
    stdio_init_all();
    set_sys_clock_133mhz();
    busy_wait_cycles_ms(500);

    bmi_init();
    INFO("BMI270 initialized\n");

    queue_init(&IMU_queue, sizeof(IMU_sample_t) * IMU_BUF_SIZE, 1);
    multicore_launch_core1(core1_entry);

    uint32_t IMU_sample_idx = 0;
    IMU_sample_t *IMUbuffer = new IMU_sample_t[IMU_BUF_SIZE];

    for (uint16_t i = 0; i < 128; i++) // Flush first readings
    {
        while (!bmi_drdy())
        {
        }
        bmi_read_FIFO(nullptr, bmi_get_FIFO_length());
    }
    while (true)
    {
        if (!bmi_drdy())
            continue;
        int16_t ax, ay, az, gx, gy, gz;
        bmi_data_t IMUdata;
        if (bmi_read_sensors(&IMUdata) == 1)
            continue; // No data to receive
        IMUbuffer[IMU_sample_idx % IMU_BUF_SIZE] = (IMU_sample_t){IMUdata,
                                                                  IMU_sample_idx++,
                                                                  0};
        if (IMU_sample_idx % IMU_BUF_SIZE == 0)
        {
            if (!queue_try_add(&IMU_queue, IMUbuffer))
                ERROR("Queue full\n");
        }
    }
}

void __time_critical_func(adc_handler)(uint16_t level)
{
    if (level < 1600)
        disconnected = true;
}

void core1_entry(void)
{

    FATFS fs;
    sd_mount(&fs);

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

    FIL IMUfile;
    char filename[32];
    sprintf(filename, "IMU-%d.bin", count);
    sd_open(&IMUfile, filename, FA_CREATE_ALWAYS | FA_WRITE);
    INFO("SD file system initiated\n");

    battery_init(5, adc_handler);

    uint16_t write_count = 0;
    IMU_sample_t *IMUbuffer = new IMU_sample_t[IMU_BUF_SIZE];
    while (getchar_timeout_us(0) != 'q' && !disconnected)
    {
        if (queue_try_remove(&IMU_queue, IMUbuffer))
        {
            // printf("%d\n", f_size(&IMUfile));
            sd_write(&IMUfile, IMUbuffer, sizeof(IMU_sample_t) * IMU_BUF_SIZE);
            if ((++write_count) % 16 == 0)
            {
                sd_sync(&IMUfile);
                INFO("Flushed file\n");
            }
        }
    }
    sd_close(&IMUfile);
    sd_unmount();
    INFO("SD card unmounted\n");

    blink();
    for (;;)
        ;
}
