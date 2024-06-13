#include <stdio.h>

#include "ff.h"
#include "f_util.h"
#include "pico/stdlib.h"
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

void blink(void);
void process_bmi_fifo(void);

int main()
{
    stdio_init_all();
    assert(set_sys_clock_khz(133000, true));
    sleep_ms(500);
    bmi_init();
    printf("BMI270 initialized\n");

    uint8_t FIFOdata[BMI_FIFO_CAP];
    for (uint16_t i = 0; i < 500; i++)
    {
        while (!bmi_drdy())
        {
        }
        bmi_read_FIFO(FIFOdata);
    }

    while (true)
    {
        if (bmi_drdy())
        {
            process_bmi_fifo();
        }
    }
    blink();
    for (;;)
        ;
}

uint32_t prev = 0;

void process_bmi_fifo(void)
{
    uint32_t t = time_us_32();
    if (t-prev > 1000000) {
        prev = t;
        printf("%d\n", bmi_get_FIFO_length());
    }
    uint8_t FIFOdata[BMI_FIFO_CAP];
    uint16_t idx = 0;
    bool at_end = false;
    uint16_t len = bmi_read_FIFO(FIFOdata);

    while (!at_end)
    {
        bmi_frame_type_t frame_type = bmi_parse_header(FIFOdata[idx]);
        // printf("%d: %s\n", frame_type, bmi_frame_type_to_string(frame_type).c_str());
        int16_t gyr_x, gyr_y, gyr_z, acc_x, acc_y, acc_z;
        switch (frame_type)
        {
        case BMI_INVALID:
            at_end = true;
            break;
        case BMI_ACC_GYR:
            bmi_parse_sensor(FIFOdata + idx + 1, &gyr_x, &gyr_y, &gyr_z);
            bmi_parse_sensor(FIFOdata + idx + 7, &acc_x, &acc_y, &acc_z);
            idx += 13;
            // printf("gx:%d,", gyr_x);
            // printf("gy:%d,", gyr_y);
            // printf("gz:%d,", gyr_z);
            // printf("ax:%d,", acc_x);
            // printf("ay:%d,", acc_y);
            // printf("az:%d\n", acc_z);
            // printf("GOOD\n");
            break;
        case BMI_GYR:
            // printf("GYR\n");
            idx += 7;
            break;
        case BMI_ACC:
            printf("ACC\n");
            idx += 7;
            break;
        case BMI_TIME:
            // printf("TIME\n"); 
            idx += 4;
            break;
        case BMI_CONFIG:
            printf("CONFIG\n");
            idx += 5;
            break;
        case BMI_SKIP:
            printf("SKIP\n");
            idx += 2;
            break;
        default:
            printf("XXXXXXXXX  %s  XXXXXXXXXX\n", bmi_frame_type_to_string(frame_type).c_str());
            // at_end = true;
            idx++;
        }
        if (idx >= len)
            at_end = true;
    }
}

void blink(void)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true)
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
}