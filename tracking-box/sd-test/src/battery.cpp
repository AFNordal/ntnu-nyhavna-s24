#include "battery.h"

static repeating_timer_t adc_timer;
static void (*adc_handler)(uint16_t);

// A repeating timer starts the adc. When the measurement is done, the provided callback is called
void battery_init(uint32_t sample_interval, void (*adc_callback)(uint16_t), alarm_pool_t *timer_pool)
{
    adc_init();

    adc_gpio_init(VBUS_PIN);
    adc_select_input(VBUS_ADC_INPUT);

    adc_irq_set_enabled(true);
    adc_fifo_setup(true, false, 1, false, false);
    adc_handler = adc_callback;
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_irq);
    irq_set_enabled(ADC_IRQ_FIFO, true);
    alarm_pool_add_repeating_timer_ms(timer_pool, sample_interval, adc_starter, NULL, &adc_timer);
}

static bool adc_starter(repeating_timer_t *rt)
{
    adc_run(true);
    return true;
}

static void adc_irq(void)
{
    adc_run(false);
    uint16_t level = adc_fifo_get();
    adc_fifo_drain();
    adc_handler(level);
    irq_clear(ADC_IRQ_FIFO);
}