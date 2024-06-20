#include "f9p.h"
static volatile uint8_t interrupt_pin;
static volatile alarm_pool_t *timer_pool;

static  uint8_t *dma_buffers[4];
static volatile int dma_channels[4];

static volatile bool buffer_newly_read[4];
static volatile bool drdy_flags[4];

void f9p_init(const uint8_t rx0_pin, const uint8_t rx1_pin, const uint8_t int_pin, alarm_pool_t *_timer_pool)
{
    uart_init(uart0, F9P_BAUDRATE);
    uart_init(uart1, F9P_BAUDRATE);
    gpio_set_function(rx0_pin, GPIO_FUNC_UART);
    gpio_set_function(rx1_pin, GPIO_FUNC_UART);

    gpio_init(int_pin);
    gpio_set_dir(int_pin, GPIO_OUT);
    timer_pool = _timer_pool;
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
        // else
        //     drdy_flags[i] = false;
    }
    // INFO("hello");

    ASSERT(!(drdy_flags[0] && drdy_flags[1]));
    ASSERT(!(drdy_flags[2] && drdy_flags[3]));
    // chan1_drdy = true;
    // if (dma_channel_get_irq1_status(dma_channels[0])) {
    //     chan0_drdy = true;
    //     chan0_buf_to_read = 0;
    //     dma_channel_acknowledge_irq1(dma_channels[0]);
    //     ASSERT(!dma_channel_get_irq1_status(dma_channels[1]));
    // } else if (dma_channel_get_irq1_status(dma_channels[1])) {
    //     chan0_drdy = true;
    //     chan0_buf_to_read = 0;
    //     dma_channel_acknowledge_irq1(dma_channels[1]);
    //     ASSERT(!dma_channel_get_irq1_status(dma_channels[0]));
    // }
    // if (dma_channel_get_irq1_status(dma_channels[2])) {
    //     chan1_drdy = true;
    //     chan1_buf_to_read = 2;
    //     dma_channel_acknowledge_irq1(dma_channels[2]);
    //     ASSERT(!dma_channel_get_irq1_status(dma_channels[3]));
    // } else if (dma_channel_get_irq1_status(dma_channels[3])) {
    //     chan1_drdy = true;
    //     chan1_buf_to_read = 3;
    //     dma_channel_acknowledge_irq1(dma_channels[3]);
    //     ASSERT(!dma_channel_get_irq1_status(dma_channels[2]));
    // }
}
static uint8_t *buf0;
static uint8_t *buf1;
static uint8_t *buf2;
static uint8_t *buf3;

static void f9p_dma_init(void)
{
    // buf0 = new uint8_t[1024];
    // int chan0 = dma_claim_unused_channel(true);
    // buf1 = new uint8_t[1024];
    // int chan1 = dma_claim_unused_channel(true);
    // buf2 = new uint8_t[1024];
    // int chan2 = dma_claim_unused_channel(true);
    // buf3 = new uint8_t[1024];
    // int chan3 = dma_claim_unused_channel(true);

    // dma_channel_config conf = dma_channel_get_default_config(chan0);
    // channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
    // channel_config_set_read_increment(&conf, false);
    // channel_config_set_write_increment(&conf, true);
    // // channel_config_set_ring(&conf, true, 10);
    // channel_config_set_dreq(&conf, uart_get_dreq(uart0, false));
    // // channel_config_set_irq_quiet(&conf, true);
    // channel_config_set_chain_to(&conf, chan1);

    // dma_channel_configure(
    //     chan0,
    //     &conf,
    //     buf0,
    //     &(uart_get_hw(uart0)->dr),
    //     1024,
    //     false);

    // conf = dma_channel_get_default_config(chan1);
    // channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
    // channel_config_set_read_increment(&conf, false);
    // channel_config_set_write_increment(&conf, true);
    // // channel_config_set_ring(&conf, true, 10);
    // channel_config_set_dreq(&conf, uart_get_dreq(uart0, false));
    // // channel_config_set_irq_quiet(&conf, true);
    // channel_config_set_chain_to(&conf, chan0);

    // dma_channel_configure(
    //     chan1,
    //     &conf,
    //     buf1,
    //     &(uart_get_hw(uart0)->dr),
    //     1024,
    //     false);

    // conf = dma_channel_get_default_config(chan2);
    // channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
    // channel_config_set_read_increment(&conf, false);
    // channel_config_set_write_increment(&conf, true);
    // // channel_config_set_ring(&conf, true, 10);
    // channel_config_set_dreq(&conf, uart_get_dreq(uart1, false));
    // // channel_config_set_irq_quiet(&conf, true);
    // channel_config_set_chain_to(&conf, chan3);

    // dma_channel_configure(
    //     chan2,
    //     &conf,
    //     buf2,
    //     &(uart_get_hw(uart1)->dr),
    //     1024,
    //     false);

    // conf = dma_channel_get_default_config(chan3);
    // channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
    // channel_config_set_read_increment(&conf, false);
    // channel_config_set_write_increment(&conf, true);
    // // channel_config_set_ring(&conf, true, 10);
    // channel_config_set_dreq(&conf, uart_get_dreq(uart1, false));
    // // channel_config_set_irq_quiet(&conf, true);
    // channel_config_set_chain_to(&conf, chan2);

    // dma_channel_configure(
    //     chan3,
    //     &conf,
    //     buf3,
    //     &(uart_get_hw(uart1)->dr),
    //     1024,
    //     false);

    // dma_channel_start(chan2);
    // dma_channel_start(chan0);
    // return;
    //////////////////////////////////////////////////////////////////////////////

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

        // dma_channels[i] = dma_claim_unused_channel(true);
        // dma_channels[i] = 2 + i;
        // dma_channel_claim(5 + i);

        dma_channel_config conf = dma_channel_get_default_config(dma_channels[i]);
        channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
        channel_config_set_read_increment(&conf, false);
        channel_config_set_write_increment(&conf, true);
        // channel_config_set_ring(&conf, true, 10);
        channel_config_set_dreq(&conf, uart_get_dreq(uart, false));
        // channel_config_set_irq_quiet(&conf, true);

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

        // if (i <= 1)
        //     dma_channel_set_irq0_enabled(dma_channels[i], true);
        // else
        //     dma_channel_set_irq1_enabled(dma_channels[i], true);
    }
    // INFO("%d\n", irq_get_exclusive_handler(DMA_IRQ_0));
    // INFO("%d\n", irq_get_exclusive_handler(DMA_IRQ_1));
    // irq_set_exclusive_handler(DMA_IRQ_0, dma_chan0_interrupt);
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler);
    // INFO("Got here");
    // irq_set_enabled(DMA_IRQ_1, true);
    irq_set_enabled(DMA_IRQ_1, true);
    dma_channel_start(dma_channels[0]);
    // dma_channel_start(dma_channels[1]);
    dma_channel_start(dma_channels[2]);
    // dma_channel_start(dma_channels[3]);
}

bool f9p_chan0_drdy(uint8_t **buf_to_read)
{
    // if ((!buffer_newly_read[0]) && (!dma_channel_is_busy(dma_channels[0])))
    // {
    //     *buf_to_read = dma_buffers[0];
    //     buffer_newly_read[0] = true;
    //     return true;
    // }
    // else if (dma_channel_is_busy(dma_channels[0]))
    // {
    //     buffer_newly_read[0] = false;
    // }
    // if ((!buffer_newly_read[1]) && (!dma_channel_is_busy(dma_channels[1])))
    // {
    //     *buf_to_read = dma_buffers[1];
    //     buffer_newly_read[1] = true;
    //     return true;
    // }
    // else if (dma_channel_is_busy(dma_channels[1]))
    // {
    //     buffer_newly_read[1] = false;
    // }
    // return false;
    if (drdy_flags[0])
    {
        drdy_flags[0] = false;
        *buf_to_read = dma_buffers[0];
        return true;
    }
    if (drdy_flags[1])
    {
        drdy_flags[1] = false;
        *buf_to_read = dma_buffers[1];
        return true;
    }
    return false;
}

bool f9p_chan1_drdy(uint8_t **buf_to_read)
{
    // if ((!buffer_newly_read[2]) && (!dma_channel_is_busy(dma_channels[2])))
    // {
    //     *buf_to_read = dma_buffers[2];
    //     buffer_newly_read[2] = true;
    //     return true;
    // }
    // else if (dma_channel_is_busy(dma_channels[2]))
    // {
    //     buffer_newly_read[2] = false;
    // }
    // if ((!buffer_newly_read[3]) && (!dma_channel_is_busy(dma_channels[3])))
    // {
    //     *buf_to_read = dma_buffers[3];
    //     buffer_newly_read[3] = true;
    //     return true;
    // }
    // else if (dma_channel_is_busy(dma_channels[3]))
    // {
    //     buffer_newly_read[3] = false;
    // }
    // return false;
    if (drdy_flags[2])
    {
        drdy_flags[2] = false;
        *buf_to_read = dma_buffers[2];
        return true;
    }
    if (drdy_flags[3])
    {
        drdy_flags[3] = false;
        *buf_to_read = dma_buffers[3];
        return true;
    }
    return false;
}

// void f9p_read_all(uint8_t *buf0, uint8_t *buf1, uint16_t *bw0, uint16_t *bw1)
// {
//     if (uart0_hw->fr & UART_UARTFR_RXFF_BITS)
//         WARNING("UART0 full");
//     if (uart1_hw->fr & UART_UARTFR_RXFF_BITS)
//         WARNING("UART1 full");

//     *bw0 = 0;
//     *bw1 = 0;
//     while (true)
//     {
//         if (uart_is_readable(uart0))
//         {
//             // INFO("Read 0\n");
//             uart_read_blocking(uart0, buf0, 1);
//             *bw0 = *bw0 + 1;
//         }
//         else if (uart_is_readable(uart1))
//         {
//             // INFO("Read 1\n");
//             uart_read_blocking(uart1, buf1, 1);
//             *bw1 = *bw1 + 1;
//         }
//         else
//         {
//             break;
//         }
//     }
//     // if(*bw1)
//     // printf("Got here %d\n", *bw1);
// }

static int64_t __time_critical_func(f9p_end_interrupt_pulse)(alarm_id_t id, void *ud)
{
    gpio_put(interrupt_pin, 0);
    return 0;
}

void __time_critical_func(f9p_send_interrupt)(void)
{
    gpio_put(interrupt_pin, 1);
    alarm_pool_add_alarm_in_ms(const_cast<alarm_pool_t *>(timer_pool), F9P_PULSE_LENGTH_MS, f9p_end_interrupt_pulse, NULL, false);
}
