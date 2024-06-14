#pragma once
#include "pico/stdlib.h"

#undef SYS_CLK_KHZ
#define SYS_CLK_KHZ 133000
#undef SYS_CLK_MHZ
#define SYS_CLK_MHZ 133

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