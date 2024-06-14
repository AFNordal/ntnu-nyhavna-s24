#pragma once
#include <stdio.h>
#include <string.h>
#include <string>

#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "bmi270_config.h"

#define BMI_FIFO_CAP 2048

#define BMI_SDA_PIN 4
#define BMI_SCL_PIN 5

#define BMI_I2C i2c0
#define BMI_I2C_ADDR 0x68

#define BMI_CHIP_ID_R 0x00

#define BMI_INIT_CTRL_R 0x59
#define BMI_INIT_DATA_R 0x5E
#define BMI_INTERNAL_STATUS_R 0x21

#define BMI_STATUS_R 0x03

#define BMI_PWR_CONF_R 0x7C
#define BMI_PWR_CTRL_R 0x7D
#define BMI_ACC_CONF_R 0x40
#define BMI_ACC_RANGE_R 0x41
#define BMI_GYR_CONF_R 0x42
#define BMI_GYR_RANGE_R 0x43

#define BMI_FIFO_DOWNS_R 0x45
#define BMI_FIFO_WTM_0_R 0x46
#define BMI_FIFO_WTM_1_R 0x47
#define BMI_FIFO_CONFIG_0_R 0x48
#define BMI_FIFO_CONFIG_1_R 0x49
#define BMI_FIFO_LENGTH_0_R 0x24
#define BMI_FIFO_LENGTH_1_R 0x25
#define BMI_FIFO_DATA_R 0x26

#define BMI_INT1_IO_CTRL_R 0x53
#define BMI_INT_MAP_DATA_R 0x58
#define BMI_INT_STATUS_0_R 0x1C
#define BMI_INT_STATUS_1_R 0x1D

#define BMI_INTERNAL_ERROR_R 0x5F
#define BMI_CMD_R 0x7E

typedef enum
{
    BMI_ACC,
    BMI_GYR,
    BMI_ACC_GYR,
    BMI_UNDEF_REGULAR,
    BMI_SKIP,
    BMI_TIME,
    BMI_CONFIG,
    BMI_UNDEF_CTRL,
    BMI_UNDEF_TYPE,
    BMI_INVALID
} bmi_frame_type_t;

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} bmi_data_t;

uint16_t bmi_get_FIFO_length(void);
void bmi_print_status(void);
void sandbox(void);
void bmi_init(void);
uint8_t bmi_read(uint8_t addr);
void bmi_write(uint8_t addr, uint8_t txdata);
void bmi_read_arr(uint8_t addr, size_t len, uint8_t *rxdata);
void bmi_write_arr(uint8_t addr, size_t len, const uint8_t *txdata);
void bmi_check_error(void);
void bmi_flush_FIFO(void);
void bmi_softreset(void);
bool bmi_drdy(void);
void bmi_read_FIFO(uint8_t *rxdata, uint16_t len);
uint8_t bmi_read_sensors(bmi_data_t *data);