#pragma once
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/irq.h"

#define VBUS_PIN 29
#define VBUS_ADC_INPUT 3

void battery_init(uint32_t sample_interval, void (*adc_callback)(uint16_t), alarm_pool_t *timer_pool);
static bool adc_starter(repeating_timer_t *rt);
static void adc_irq(void);