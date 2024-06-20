#pragma once
#include "pico/stdlib.h"
#include <stdio.h>

#undef SYS_CLK_KHZ
#define SYS_CLK_KHZ 125000
#undef SYS_CLK_MHZ
#define SYS_CLK_MHZ 125

#ifndef NDEBUG
#define INFO(...)                                     \
                                                      \
    {                                                 \
        printf("INFO: "); \
        printf(__VA_ARGS__);                          \
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
#define ERROR(...)                                      \
                                                        \
    {                                                   \
        printf("%s:%d: ERROR: \n", __FILE__, __LINE__); \
        printf(__VA_ARGS__);                            \
        while (1)                                       \
        {                                               \
        }                                               \
    }
#else
#define ERROR(...) ;
#endif

#ifndef NDEBUG
#define ASSERT(x)                                               \
    if (!(x))                                                   \
    {                                                           \
        ERROR("%s:%d: Assertion failed\n", __FILE__, __LINE__); \
    }
#else
#define ASSERT(x) ;
#endif

inline void set_sys_clock_133mhz(void)
{
    set_sys_clock_khz(SYS_CLK_KHZ, true);
}

inline void busy_wait_cycles_us(uint32_t us)
{
    busy_wait_at_least_cycles(us * SYS_CLK_MHZ);
}

inline void busy_wait_cycles_ms(uint32_t ms)
{
    busy_wait_at_least_cycles(ms * SYS_CLK_KHZ);
}

inline void blink(void)
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    while (true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        busy_wait_cycles_ms(10);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        busy_wait_cycles_ms(50);
    }
}