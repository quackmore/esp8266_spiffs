/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */

extern "C"
{
#include "user_interface.h"
#include "mem.h"
}
#include "esp8266_spiffs.hpp"

class flafs_test : public flafs
{
  public:
    void test(void);
};

void ICACHE_FLASH_ATTR flafs_test::test(void)
{
    // FS_START
    // SYSTEM_PARTITION_RF_CAL_ADDR
    P_INFO("[INFO]: TEST\n");
    P_INFO("[INFO]: TEST\n");
    P_DEBUG("[DEBUG]: Available heap size: %d\n", system_get_free_heap_size());

    // warning: using stack instead of heap will produce hallucinations
    uint32 two_sect_buffer_space = (uint32)os_zalloc((1024 * 4 * 2) + 4); // two sector size
    P_TRACE("[TRACE]: buffer unaligned address: %X\n", two_sect_buffer_space);
    u8_t *two_sect_buffer = (u8_t *)(((two_sect_buffer_space + 4) / 4) * 4);
    P_TRACE("[TRACE]: buffer aligned address: %X\n", two_sect_buffer);
    s32_t res;

    // erase first and second available sector
    P_INFO("[INFO]: Preparing for flash memory function test...\n");
    P_TRACE("[TRACE]: erasing first and second available sector\n");
    res = spiffs_erase(FS_START, (1024 * 5));

    // read first and second available sector
    P_TRACE("[TRACE]: reading first and second available sector\n");
    res = spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);

    // check that they are filled with 0xFF
    {
        int idx;
        for (idx = 0; idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: flash was not erased!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
    }
    P_INFO("[INFO]: Test 1 - write 15 bytes to flash (aligned to the beginning of sector)\n");
    {
        int idx;
        char str[15];
        for (idx = 0; idx < 15; idx++)
            str[idx] = idx;
        res = spiffs_write(FS_START, 15, (u8_t *)str);
        res = spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
        for (idx = 0; idx < 15; idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not written!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = 16; idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
    }
    P_INFO("[INFO]: Test 1 successfully completed\n");
    P_INFO("[INFO]: Test 2 - write 15 bytes to flash (unaligned at start + 21)\n");
    {
        int idx;
        char str[15];
        for (idx = 0; idx < 15; idx++)
            str[idx] = 21 + idx;
        res = spiffs_write(FS_START + 21, 15, (u8_t *)str);
        res = spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
        for (idx = 0; idx < 15; idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = 16; idx < 21; idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = 21; idx < (21 + 15); idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not written!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = (21 + 15); idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
    }
    P_INFO("[INFO]: Test 2 successfully completed\n");
    P_INFO("[INFO]: Test 3 - write 299 bytes to flash (unaligned at start + 4051) and crossig sector boundary\n");
    {
        int idx;
        char str[299];
        for (idx = 0; idx < 299; idx++)
            str[idx] = 0xAA;
        res = spiffs_write(FS_START + 4051, 299, (u8_t *)str);
        res = spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
        for (idx = 0; idx < 15; idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = 16; idx < 21; idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = 21; idx < (21 + 15); idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = (21 + 15); idx < 4051; idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        for (idx = 4051; idx < (4051 + 299); idx++)
        {
            if (two_sect_buffer[idx] != 0xAA)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not written!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
        }
        for (idx = (4051 + 299); idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
    }
    P_INFO("[INFO]: Test 3 successfully completed\n");
    P_INFO("[INFO]: Test 4 - erase first two sectors\n");
    {
        res = spiffs_erase(FS_START, (1024 * 5));
        res = spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);

        // check that they are filled with 0xFF
        int idx;
        for (idx = 0; idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not erased!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return;
            }
    }
    P_INFO("[INFO]: Test 4 successfully completed\n");
    P_INFO("[INFO]: Test 5 - boundary checks\n");
    {
        res = spiffs_read(FS_START - 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_read(FS_END + 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_read(FS_END - 4, 8, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_write(FS_START - 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_write(FS_END + 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_write(FS_END - 4, 8, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_erase(FS_START - 1, 1024 * 4 * 2);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_erase(FS_END + 1, 1024 * 4 * 2);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = spiffs_erase(FS_END - 4, 8);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
    }
    P_INFO("[INFO]: Test 5 successfully completed\n");
    P_INFO("[INFO]: That's all.\n");
    os_free((void *)two_sect_buffer_space);
    P_DEBUG("[DEBUG]: Available heap size: %d\n", system_get_free_heap_size());
}

flafs first_instance;

void ICACHE_FLASH_ATTR run_tests(void)
{
    flafs_test fs;
    fs.test();
}