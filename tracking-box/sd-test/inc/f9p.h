#pragma once
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "sys_utils.h"

#define F9P_PULSE_LENGTH_MS 1
#define F9P_BAUDRATE 912600
#define F9P_BUF_SIZE 4096*2

void f9p_init(const uint8_t rx0_pin, const uint8_t rx1_pin, const uint8_t int_pin, alarm_pool_t *_timer_pool);
void f9p_read_all(uint8_t *buf0, uint8_t *buf1, uint16_t *bw0, uint16_t *bw1);
void __time_critical_func(f9p_send_interrupt)(void);
static void f9p_dma_init(void);
bool f9p_chan0_drdy(uint8_t **buf_to_read);
bool f9p_chan1_drdy(uint8_t **buf_to_read);
