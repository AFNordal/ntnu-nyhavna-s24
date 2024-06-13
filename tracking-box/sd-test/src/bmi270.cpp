#include "bmi270.h"

void bmi_init(void)
{
    i2c_init(BMI_I2C, 1000 * 1000);
    gpio_set_function(BMI_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BMI_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BMI_SDA_PIN);
    gpio_pull_up(BMI_SCL_PIN);

    // Reset all registers
    sleep_ms(10);
    bmi_softreset();
    sleep_ms(10);

    bmi_write(BMI_PWR_CONF_R, (0x00 << 0) | (0x01 << 1));

    // Load config file
    bmi_write(BMI_INIT_CTRL_R, 0x00);
    bmi_write_arr(BMI_INIT_DATA_R, sizeof(bmi270_config_file), bmi270_config_file);
    bmi_write(BMI_INIT_CTRL_R, 0x01);
    sleep_ms(20);

    uint8_t init_status = bmi_read(BMI_INTERNAL_STATUS_R);
    if ((init_status & 0x0F) != 0x1)
    {
        printf("BMI270 INITIALIZATION FAILED\n");
        printf("Error code: %x", init_status);
        while (true)
        {
        }
    }
    // Acc: 1.6KHz ODR, normal filtering
    bmi_write(BMI_ACC_CONF_R, (0x0c << 0) | (0x02 << 4) | (0x01 << 7));
    // Gyro: 1.6KHz ODR, normal filtering, low-noise
    bmi_write(BMI_GYR_CONF_R, (0x0c << 0) | (0x02 << 4) | (0x01 << 6) | (0x01 << 7));
    // Acc: +/- 2g
    bmi_write(BMI_ACC_RANGE_R, 0x00);
    // Gyro: +/-250 dps, 131.2 LSB/dps
    bmi_write(BMI_GYR_RANGE_R, (0x03 << 0) | (0x00 << 3));

    // Gyro downsample by 4 to get 1.6KHz, unifltered data, no acc downsample
    bmi_write(BMI_FIFO_DOWNS_R, (0x02 << 0) | (0x00 << 3) | (0x00 << 4) | (0x00 << 7));
    // FIFO watermark 2 bytes
    bmi_write(BMI_FIFO_WTM_0_R, 0x02);
    bmi_write(BMI_FIFO_WTM_1_R, 0x00);

    // Map watermark to INT1
    bmi_write(BMI_INT_MAP_DATA_R, (0x01 << 1));
    // Enable INT1 output, active high
    bmi_write(BMI_INT1_IO_CTRL_R, (0x01 << 1) | (0x01 << 3));

    // Enable sensortime frames
    bmi_write(BMI_FIFO_CONFIG_0_R, (0x01 << 1));
    // Enable FIFO header (for sensortime), enable acc and gyr data to FIFO
    bmi_write(BMI_FIFO_CONFIG_1_R, (0x01 << 4) | (0x01 << 6) | (0x01 << 7));

    bmi_check_error();

    // Enable gyro and acc
    bmi_write(BMI_PWR_CTRL_R, (0x01 << 1) | (0x01 << 2));
    // bmi_flush_FIFO();
}

uint16_t bmi_get_FIFO_length(void)
{
    uint16_t len;
    bmi_read_arr(BMI_FIFO_LENGTH_0_R, 2, (uint8_t *)&len);
    return len;
}

void bmi_print_status(void)
{
    uint8_t statusdata = bmi_read(BMI_STATUS_R);
    printf("%x\n", statusdata);
}

void bmi_read_arr(uint8_t addr, size_t len, uint8_t *rxdata)
{
    i2c_write_blocking(BMI_I2C, BMI_I2C_ADDR, &addr, 1, false);
    size_t bytes_read = i2c_read_blocking(BMI_I2C, BMI_I2C_ADDR, rxdata, len, false);
    if (bytes_read != len)
    {
        printf("BMI I2C READ ERROR\n");
    }
}

void bmi_write_arr(uint8_t addr, size_t len, const uint8_t *txdata)
{
    uint8_t txarr[len + 1];
    txarr[0] = addr;
    // memcpy(txarr + 1, txdata, len);
    for (int i = 0; i < len; i++)
    {
        txarr[i + 1] = txdata[i];
    }
    // printf("%x\n", txarr[len]);
    size_t bytes_written = i2c_write_blocking(BMI_I2C, BMI_I2C_ADDR, txarr, len + 1, false);
    if (bytes_written != len + 1)
    {
        printf("BMI I2C WRITE ERROR; wrote %d bytes.\n", bytes_written);
    }
    sleep_us(450);
}

void bmi_write(uint8_t addr, uint8_t txdata)
{
    uint8_t txarr[] = {addr, txdata};
    size_t bytes_written = i2c_write_blocking(BMI_I2C, BMI_I2C_ADDR, txarr, 2, false);
    if (bytes_written != 2)
    {
        printf("BMI I2C WRITE ERROR; wrote %d bytes.\n", bytes_written);
    }
    sleep_us(450);
}

uint8_t bmi_read(uint8_t addr)
{
    uint8_t rxdata;
    i2c_write_blocking(BMI_I2C, BMI_I2C_ADDR, &addr, 1, false);
    if (i2c_read_blocking(BMI_I2C, BMI_I2C_ADDR, &rxdata, 1, false) != 1)
    {
        printf("BMI I2C READ ERROR\n");
        return -1;
    }
    else
    {
        return rxdata;
    }
}

void bmi_check_error(void)
{
    uint8_t errordata = bmi_read(BMI_INTERNAL_ERROR_R);
    if ((errordata & (1 << 2)) != 0)
    {
        printf("INTERNAL BMI ERROR: Fatal\n");
    }
    else if ((errordata & (1 << 1)) != 0)
    {
        printf("INTERNAL BMI ERROR: Long processing time\n");
    }
}

void bmi_flush_FIFO(void)
{
    bmi_write(BMI_CMD_R, 0xb0);
}

void bmi_softreset(void)
{
    bmi_write(BMI_CMD_R, 0xb6);
}

void sandbox(void)
{
}

// Checks if watermark interrupt is triggered
bool bmi_drdy(void)
{
    uint8_t intstatus = bmi_read(BMI_INT_STATUS_1_R);
    return bool(intstatus);
}

uint16_t bmi_read_FIFO(uint8_t *rxdata)
{
    uint16_t len = 64; // bmi_get_FIFO_length();
    bmi_read_arr(BMI_FIFO_DATA_R, len, rxdata);
    return len;
}

std::string bmi_frame_type_to_string(bmi_frame_type_t f)
{
    switch (f)
    {
    case BMI_ACC:
        return "acc data";
    case BMI_GYR:
        return "gyro data";
    case BMI_ACC_GYR:
        return "acc and gyro data";
    case BMI_UNDEF_REGULAR:
        return "undefined regular";
    case BMI_SKIP:
        return "skip";
    case BMI_TIME:
        return "time";
    case BMI_CONFIG:
        return "config";
    case BMI_UNDEF_CTRL:
        return "undefined control";
    case BMI_UNDEF_TYPE:
        return "undefined frame type";
    case BMI_INVALID:
        return "invalid";
    default:
        return "NOT A TYPE";
    }
}

bmi_frame_type_t bmi_parse_header(uint8_t h)
{
    if (h == 0x80)
    {
        return BMI_INVALID;
    }
    uint8_t mode = ((h >> 6) & 0x03);
    uint8_t parm = ((h >> 2) & 0x0F);
    switch (mode)
    {
    case 1:
        switch (parm)
        {
        case 0:
            return BMI_SKIP;
        case 1:
            return BMI_TIME;
        case 2:
            return BMI_CONFIG;
        default:
            return BMI_UNDEF_CTRL;
        }
        break;
    case 2:
        // printf("%X\n", parm);
        if ((parm & 0x07) == 0x03)
            return BMI_ACC_GYR;
        else if (parm & 0x01)
            return BMI_ACC;
        else if (parm & 0x02)
            return BMI_GYR;
        else
            return BMI_UNDEF_REGULAR;
    default:
        return BMI_UNDEF_TYPE;
    }
}

void bmi_parse_sensor(uint8_t *FIFOdata, int16_t *x, int16_t *y, int16_t *z)
{
    *x = (int16_t)((FIFOdata[1] << 8) | FIFOdata[0]);
    *y = (int16_t)((FIFOdata[3] << 8) | FIFOdata[2]);
    *z = (int16_t)((FIFOdata[5] << 8) | FIFOdata[4]);
}