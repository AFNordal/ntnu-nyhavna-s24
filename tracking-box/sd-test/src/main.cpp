#include <stdio.h>

#include "ff.h"
#include "f_util.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hw_config.h"
#include "sd_card.h"
#include "bmi270.h"

#define SYS_CLK 133000

#undef assert
#define assert(x)                                                     \
    if (!(x))                                                         \
    {                                                                 \
        printf("Assertion failed, line %s:%d\n", __FILE__, __LINE__); \
        while (1)                                                     \
        {                                                             \
        }                                                             \
    }

void blink(void);
void process_bmi_fifo(void);
void core1_entry(void);

int main()
{
    stdio_init_all();
    assert(set_sys_clock_khz(SYS_CLK_KHZ, true));
    sleep_ms(500);

    multicore_launch_core1(blink);

    bmi_init();
    printf("BMI270 initialized\n");

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
                // printf("gx:%d,", gx);
                // printf("gy:%d,", gy);
                // printf("gz:%d,", gz);
                // printf("ax:%d,", ax);
                // printf("ay:%d,", ay);
                // printf("az:%d\n", az);
            }
        }
    }
}

void core1_entry(void) {
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
        busy_wait_at_least_cycles(1000*SYS_CLK);
        gpio_put(LED_PIN, 0);
        busy_wait_at_least_cycles(1000*SYS_CLK);
    }
}