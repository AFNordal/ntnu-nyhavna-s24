#include "battery.h"

static struct repeating_timer adc_timer;
static void (*adc_handler)(uint16_t);

void battery_init(uint32_t sample_interval, void (*fun)(uint16_t))
{
    adc_init();

    gpio_init(VBAT_PIN);
    gpio_set_dir(VBAT_PIN, GPIO_IN);

    adc_gpio_init(VBUS_PIN);
    adc_select_input(VBUS_ADC_INPUT);

    adc_irq_set_enabled(true);
    adc_fifo_setup(true, false, 1, false, false);
    adc_handler = fun;
    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_irq);
    irq_set_enabled(ADC_IRQ_FIFO, true);
    add_repeating_timer_ms(sample_interval, adc_starter, NULL, &adc_timer);
}


static bool __time_critical_func(adc_starter)(struct repeating_timer *rt)
{
    // printf("Hello %d\n", adc_fifo_get_level());
    // adc_irq_set_enabled(true);
    adc_run(true);
    return true;
}

static void __time_critical_func(adc_irq)(void) {
    // printf("Bingbong\n");
    adc_run(false);
    uint16_t level = adc_fifo_get();
    adc_fifo_drain();
    adc_handler(level);
    irq_clear(ADC_IRQ_FIFO);
}