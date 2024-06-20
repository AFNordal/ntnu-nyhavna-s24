#include "sd_utils.h"

void sd_mount(FATFS *fs)
{
    FRESULT fr = f_mount(fs, "", 1);
    if (FR_OK != fr)
        ERROR("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
}

bool sd_open(FIL *file, const char *const filename, BYTE mode)
{
    FRESULT fr = f_open(file, filename, mode);
    if (FR_OK != fr && FR_EXIST != fr)
    {
        ERROR("f_open error: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return 1;
    }
    else
        return 0;
}

void sd_rm(const char *const filename)
{
    FRESULT fr = f_unlink(filename);
    if (FR_OK != fr && FR_EXIST != fr)
        WARNING("f_unlink error: %s (%d)\n", filename, FRESULT_str(fr), fr);
}

void sd_sync(FIL *file)
{
    FRESULT fr = f_sync(file);
    if (FR_OK != fr)
        WARNING("f_sync error: %s (%d)\n", FRESULT_str(fr), fr);
}

size_t sd_write(FIL *file, const void *buffer, UINT len)
{
    UINT n;
    FRESULT fr = f_write(file, buffer, len, &n);
    if (FR_OK != fr)
        FATAL("f_write error: %s (%d)\n", FRESULT_str(fr), fr);
    return n;
}

void sd_close(FIL *file)
{
    FRESULT fr = f_close(file);
    if (FR_OK != fr)
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
}

void sd_unmount(void)
{
    FRESULT fr = f_unmount("");
    if (FR_OK != fr)
        ERROR("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
}

void sd_rewind(FIL *file)
{
    FRESULT fr = f_truncate(file);
    if (FR_OK != fr)
        ERROR("f_truncate error: %s (%d)\n", FRESULT_str(fr), fr);
    fr = f_rewind(file);
    if (FR_OK != fr)
        ERROR("f_rewind error: %s (%d)\n", FRESULT_str(fr), fr);
}

uint8_t sd_readnum(FIL *file, uint32_t *num)
{
    char buf[32];
    if (f_gets(buf, 32, file))
    {
        *num = strtol(buf, nullptr, 10);
        return PICO_OK;
    }
    else
        return PICO_ERROR_GENERIC;
}

void sd_writenum(FIL *file, uint32_t num)
{
    if (f_printf(file, "%d\n", num) <= 0)
        ERROR("f_printf error\n");
}