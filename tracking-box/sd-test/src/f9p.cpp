#include "f9p.h"
static uint8_t interrupt_pin;
static alarm_pool_t *timer_pool;

// Must be aligned for DMA ring buffer to work
alignas(F9P_BUF_SIZE) static uint8_t dma_buffer_aligned[4 * F9P_BUF_SIZE];
static uint8_t *dma_buffer_pointers[4];
static int dma_channels[4];

// Indicate that a dma_buffer is full and ready to read
static volatile bool drdy_flags[4];

void f9p_init(const uint8_t rx0_pin, const uint8_t rx1_pin, const uint8_t int_pin, alarm_pool_t *_timer_pool)
{
    uart_init(uart0, F9P_BAUDRATE);
    uart_init(uart1, F9P_BAUDRATE);
    gpio_set_function(rx0_pin, GPIO_FUNC_UART);
    gpio_set_function(rx1_pin, GPIO_FUNC_UART);

    gpio_init(int_pin);
    gpio_set_dir(int_pin, GPIO_OUT);
    interrupt_pin = int_pin;
    timer_pool = _timer_pool;
    // __sync_synchronize used to sync non-volatile variables
    __sync_synchronize();
    f9p_dma_init();
    __sync_synchronize();
}

static void dma_handler(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (dma_channel_get_irq1_status(dma_channels[i]))
        {
            drdy_flags[i] = true;
            dma_channel_acknowledge_irq1(dma_channels[i]);
            uint8_t inactive_dma_idx = ((i == 0) || (i == 2)) ? (i + 1) : (i - 1);
            if (drdy_flags[inactive_dma_idx])
            {
                drdy_flags[inactive_dma_idx] = false;
                WARNING("DRDY flags not reset in time\n");
            }
        }
    }
}

// dma_channels idx 0 and 1 take turns reading from uart0 into their buffer, and
// idx 2 and 3 read from uart1 similarly. Each DMA triggers dma_handler and starts
// the other DMA when its buffer is full.
static void f9p_dma_init(void)
{
    for (int i = 0; i < 4; i++)
    {
        dma_channels[i] = dma_claim_unused_channel(true);
        // dma_buffers point into different partitions of dma_buffer_aligned
        dma_buffer_pointers[i] = dma_buffer_aligned + i * F9P_BUF_SIZE;
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
        channel_config_set_ring(&conf, true, trailing_0s(F9P_BUF_SIZE));
        if ((i == 0) || (i == 2))
            channel_config_set_chain_to(&conf, dma_channels[i + 1]);
        else
            channel_config_set_chain_to(&conf, dma_channels[i - 1]);

        dma_channel_configure(
            dma_channels[i],
            &conf,
            dma_buffer_pointers[i],
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

// Return whether a buffer is ready to be read, and output said buffer
bool f9p_chan0_drdy(uint8_t **buf_to_read)
{
    if (drdy_flags[0])
    {
        drdy_flags[0] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffer_pointers[0];
        return true;
    }
    if (drdy_flags[1])
    {
        drdy_flags[1] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffer_pointers[1];
        return true;
    }
    return false;
}

// Return whether a buffer is ready to be read, and output said buffer
bool f9p_chan1_drdy(uint8_t **buf_to_read)
{
    if (drdy_flags[2])
    {
        drdy_flags[2] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffer_pointers[2];
        return true;
    }
    if (drdy_flags[3])
    {
        drdy_flags[3] = false;
        __sync_synchronize();
        *buf_to_read = dma_buffer_pointers[3];
        return true;
    }
    return false;
}

static int64_t f9p_end_interrupt_pulse(alarm_id_t id, void *ud)
{
    gpio_put(interrupt_pin, 0);
    return 0;
}

void f9p_send_interrupt(void)
{
    gpio_put(interrupt_pin, 1);
    alarm_pool_add_alarm_in_ms(timer_pool, F9P_PULSE_LENGTH_MS, f9p_end_interrupt_pulse, NULL, false);
}

// Returns log2 of a power of 2
static uint8_t trailing_0s(uint32_t num) {
    uint8_t c = 32;
    num &= -signed(num);
    if (num) c--;
    if (num & 0x0000FFFF) c -= 16;
    if (num & 0x00FF00FF) c -= 8;
    if (num & 0x0F0F0F0F) c -= 4;
    if (num & 0x33333333) c -= 2;
    if (num & 0x55555555) c -= 1;
    return c;
}