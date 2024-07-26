#include "f9p.h"
static volatile uint8_t interrupt_pin;
// static volatile alarm_pool_t *timer_pool;

static uint8_t *dma_buffers[4];
static volatile int dma_channels[4];

static volatile bool buffer_newly_read[4];
static volatile bool drdy_flags[4];

void f9p_init(const uint8_t rx0_pin, const uint8_t rx1_pin, const uint8_t int_pin)//, alarm_pool_t *_timer_pool)
{
    uart_init(uart0, F9P_BAUDRATE);
    uart_init(uart1, F9P_BAUDRATE);
    gpio_set_function(rx0_pin, GPIO_FUNC_UART);
    gpio_set_function(rx1_pin, GPIO_FUNC_UART);

    gpio_init(int_pin);
    gpio_set_dir(int_pin, GPIO_OUT);
    // timer_pool = _timer_pool;
    interrupt_pin = int_pin;
    // timer_pool and interrupt_pin are not volatile
    __sync_synchronize();
    f9p_dma_init();
}

static void dma_handler(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (dma_channel_get_irq1_status(dma_channels[i]))
        {
            drdy_flags[i] = true;
            dma_channel_acknowledge_irq1(dma_channels[i]);
        }
    }

    // Only one DMA should be active for each uart
    ASSERT(!(drdy_flags[0] && drdy_flags[1]));
    ASSERT(!(drdy_flags[2] && drdy_flags[3]));
}

static void f9p_dma_init(void)
{

    for (int i = 0; i < 4; i++)
    {
        dma_channels[i] = dma_claim_unused_channel(true);
        dma_buffers[i] = new uint8_t[F9P_BUF_SIZE];
    }
    for (int i = 0; i < 4; i++)
    {
        uart_inst_t *uart;
        if (i <= 1)
            uart = uart0;
        else
            uart = uart1;


        dma_channel_config conf = dma_channel_get_default_config(dma_channels[i]);
        channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
        channel_config_set_read_increment(&conf, false);
        channel_config_set_write_increment(&conf, true);
        channel_config_set_dreq(&conf, uart_get_dreq(uart, false));

        if ((i == 0) || (i == 2))
            channel_config_set_chain_to(&conf, dma_channels[i + 1]);
        else
            channel_config_set_chain_to(&conf, dma_channels[i - 1]);

        dma_channel_configure(
            dma_channels[i],
            &conf,
            dma_buffers[i],
            &(uart_get_hw(uart)->dr),
            F9P_BUF_SIZE,
            false);

        dma_channel_set_irq1_enabled(dma_channels[i], true);
    }
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler);
    irq_set_enabled(DMA_IRQ_1, true);
    dma_channel_start(dma_channels[0]);
    dma_channel_start(dma_channels[2]);
}

bool f9p_chan0_drdy(uint8_t **buf_to_read)
{
    if (drdy_flags[0])
    {
        drdy_flags[0] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffers[0];
        return true;
    }
    if (drdy_flags[1])
    {
        drdy_flags[1] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffers[1];
        return true;
    }
    return false;
}

bool f9p_chan1_drdy( uint8_t **buf_to_read)
{
    if (drdy_flags[2])
    {
        drdy_flags[2] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffers[2];
        return true;
    }
    if (drdy_flags[3])
    {
        drdy_flags[3] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffers[3];
        return true;
    }
    return false;
}

static int64_t __time_critical_func(f9p_end_interrupt_pulse)(alarm_id_t id, void *ud)
{
    gpio_put(interrupt_pin, 0);
    return 0;
}

void __time_critical_func(f9p_send_interrupt)(void)
{
    gpio_put(interrupt_pin, 1);
    for (int i = 0; i < 1000; i++)
        tight_loop_contents();
    gpio_put(interrupt_pin, 0);
    // alarm_pool_add_alarm_in_ms(const_cast<alarm_pool_t *>(timer_pool), F9P_PULSE_LENGTH_MS, f9p_end_interrupt_pulse, NULL, false);
}
