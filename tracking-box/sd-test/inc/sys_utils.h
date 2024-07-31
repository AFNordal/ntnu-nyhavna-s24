#pragma once
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include <stdio.h>

#undef SYS_CLK_KHZ
#define SYS_CLK_KHZ 133000
#undef SYS_CLK_MHZ
#define SYS_CLK_MHZ 133

#ifndef NDEBUG
#define INFO(...)                                                                                                        \
                                                                                                                         \
    {                                                                                                                    \
        uint32_t _info_t_ = time_us_32();                                                                                \
        printf("[%d:%d.%d] INFO: ", _info_t_ / 60000000, (_info_t_ % 60000000) / 1000000, (_info_t_ % 1000000) / 10000); \
        printf(__VA_ARGS__);                                                                                             \
    }
#else
#define INFO(...) ;
#endif

#ifndef NDEBUG
#define WARNING(...)                                     \
                                                         \
    {                                                    \
        printf("%s:%d: WARNING:\n", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                             \
    }
#else
#define WARNING(...) ;
#endif

#ifndef NDEBUG
#define ERROR(...)                                     \
                                                       \
    {                                                  \
        printf("%s:%d ERROR: \n", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                           \
    }
#else
#define ERROR(...) ;
#endif

#ifndef NDEBUG
#define FATAL(...)                                     \
    {                                                  \
        printf("%s:%d FATAL: \n", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                           \
    }
#else
#define FATAL(...)   \
    {                \
        err_blink(); \
    }
#endif

#ifndef NDEBUG
#define ASSERT(x)                                        \
    if (!(x))                                            \
    {                                                    \
        ERROR("Assertion failed\n", __FILE__, __LINE__); \
    }
#else
#define ASSERT(x) ;
#endif

inline void set_sys_clock_133mhz(void)
{
    set_sys_clock_khz(SYS_CLK_KHZ, true);
}

// Builtin sleep functions don't work on multicore
inline void busy_wait_cycles_us(uint32_t us)
{
    busy_wait_at_least_cycles(us * SYS_CLK_MHZ);
}

// Builtin sleep functions don't work on multicore
inline void busy_wait_cycles_ms(uint32_t ms)
{
    busy_wait_at_least_cycles(ms * SYS_CLK_KHZ);
}

inline void led_init(void)
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

// Fast blink, 16% duty cycle
inline void info_blink(void)
{
    led_init();
    while (true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        busy_wait_cycles_ms(10);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        busy_wait_cycles_ms(50);
    }
}

// Fast blink, 50% duty cycle
inline void err_blink(void)
{
    led_init();
    while (true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        busy_wait_cycles_ms(20);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        busy_wait_cycles_ms(20);
    }
}

// Controls the onboard LED, blinking once for each core that is alive and running
inline void heartbeat_blink(queue_t *heartbeat_queue)
{
    static uint32_t prev_hb = 0;
    static uint32_t prev_on = 0;
    static uint32_t prev_period = 0;
    static uint8_t blink_count = 0;
    static bool is_on = false;

    uint32_t t = time_us_32();
    if (queue_try_remove(heartbeat_queue, nullptr))
    {
        prev_hb = t;
    }
    bool other_alive = (t - prev_hb < 5000);

    if (is_on && (t - prev_on > 50 * 1000))
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        is_on = false;
    }
    else if ((!is_on) && (blink_count < 2) && (t - prev_on > 100 * 1000) && other_alive)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        prev_on = t;
        is_on = true;
        blink_count++;
    }
    else if ((!is_on) && (t - prev_period > 1000 * 1000))
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        prev_on = t;
        prev_period = t;
        is_on = true;
        blink_count = 1;
    }
}