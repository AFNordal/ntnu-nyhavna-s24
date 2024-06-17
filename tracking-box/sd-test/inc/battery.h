#pragma once
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/irq.h"

#define VBAT_PIN 23
#define VBUS_PIN 29
#define VBUS_ADC_INPUT 3

void battery_init(uint32_t sample_interval, void (*fun)(uint16_t));
static bool __time_critical_func(adc_starter)(struct repeating_timer *rt);
static void __time_critical_func(adc_irq)(void);