#pragma once
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hw_config.h"
#include "hw_config.h"
#include "ff.h"
#include "f_util.h"
#include "sd_card.h"
#include "sys_utils.h"

void sd_mount(FATFS *fs);
bool sd_open(FIL *file, const char *filename, BYTE mode);
void sd_rm(const char *const filename);
void sd_sync(FIL *file);
size_t sd_write(FIL *file, const void *buffer, UINT len);
size_t sd_printf(FIL *file, const char *str, ...);
void sd_close(FIL *file);
void sd_unmount(void);
void sd_rewind(FIL *file);
uint8_t sd_readnum(FIL *file, uint32_t *num);
void sd_writenum(FIL *file, uint32_t num);