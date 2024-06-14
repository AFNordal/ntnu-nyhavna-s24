#include <stdio.h>

#include "ff.h"
#include "f_util.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "clock_config.h"
#include "hw_config.h"
#include "sd_card.h"
#include "bmi270.h"


#undef assert
#define assert(x)                                                     \
    if (!(x))                                                         \
    {                                                                 \
        printf("Assertion failed, line %s:%d\n", __FILE__, __LINE__); \
        while (1)                                                     \
        {                                                             \
        }                                                             \
    }


queue_t imu_queue;

void blink(void);
void process_bmi_fifo(void);
void core1_entry(void);

int main()
{
    stdio_init_all();
    set_sys_clock_133mhz();
    busy_wait_cycles_ms(500);


    bmi_init();
    printf("BMI270 initialized\n");

    queue_init(&imu_queue, sizeof(bmi_data_t), 64);
    multicore_launch_core1(core1_entry);

    for (uint16_t i = 0; i < 100; i++)
    {
        while (!bmi_drdy())
        {
        }
        bmi_read_FIFO(nullptr, bmi_get_FIFO_length());
    }

    while (true)
    {
        if (bmi_drdy())
        {
            int16_t ax, ay, az, gx, gy, gz;
            bmi_data_t bmidata;
            if (bmi_read_sensors(&bmidata) == 0)
            {
                printf("Added\n");
                queue_add_blocking(&imu_queue, &bmidata);
            }
        }
    }
}

void core1_entry(void) {
    while (true)
    {
        // printf("removing\n");
        bmi_data_t bmidata;
        queue_remove_blocking(&imu_queue, &bmidata);
        print_bmi_data(&bmidata);
    }
    
    blink();
    for (;;)
        ;
}


void blink(void)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true)
    {
        gpio_put(LED_PIN, 1);
        busy_wait_cycles_ms(500);
        gpio_put(LED_PIN, 0);
        busy_wait_cycles_ms(500);
    }
}