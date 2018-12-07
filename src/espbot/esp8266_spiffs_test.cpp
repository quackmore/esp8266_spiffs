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

int ICACHE_FLASH_ATTR flash_function_test(void)
{
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
    res = esp_spiffs_erase(FS_START, (1024 * 5));

    // read first and second available sector
    P_TRACE("[TRACE]: reading first and second available sector\n");
    res = esp_spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);

    // check that they are filled with 0xFF
    {
        int idx;
        for (idx = 0; idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: flash was not erased!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
    }
    P_INFO("[INFO]: Test 1 - write 15 bytes to flash (aligned to the beginning of sector)\n");
    {
        int idx;
        char str[15];
        for (idx = 0; idx < 15; idx++)
            str[idx] = idx;
        res = esp_spiffs_write(FS_START, 15, (u8_t *)str);
        res = esp_spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
        for (idx = 0; idx < 15; idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not written!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = 16; idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
    }
    P_INFO("[INFO]: Test 1 successfully completed\n");
    P_INFO("[INFO]: Test 2 - write 15 bytes to flash (unaligned at start + 21)\n");
    {
        int idx;
        char str[15];
        for (idx = 0; idx < 15; idx++)
            str[idx] = 21 + idx;
        res = esp_spiffs_write(FS_START + 21, 15, (u8_t *)str);
        res = esp_spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
        for (idx = 0; idx < 15; idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = 16; idx < 21; idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = 21; idx < (21 + 15); idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not written!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = (21 + 15); idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
    }
    P_INFO("[INFO]: Test 2 successfully completed\n");
    P_INFO("[INFO]: Test 3 - write 299 bytes to flash (unaligned at start + 4051 and crossing sector boundary)\n");
    {
        int idx;
        char str[299];
        for (idx = 0; idx < 299; idx++)
            str[idx] = 0xAA;
        res = esp_spiffs_write(FS_START + 4051, 299, (u8_t *)str);
        res = esp_spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
        for (idx = 0; idx < 15; idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = 16; idx < 21; idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = 21; idx < (21 + 15); idx++)
            if (two_sect_buffer[idx] != idx)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = (21 + 15); idx < 4051; idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        for (idx = 4051; idx < (4051 + 299); idx++)
        {
            if (two_sect_buffer[idx] != 0xAA)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not written!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
        }
        for (idx = (4051 + 299); idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was written on wrong address!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
    }
    P_INFO("[INFO]: Test 3 successfully completed\n");
    P_INFO("[INFO]: Test 4 - erase first two sectors\n");
    {
        res = esp_spiffs_erase(FS_START, (1024 * 5));
        res = esp_spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);

        // check that they are filled with 0xFF
        int idx;
        for (idx = 0; idx < (1024 * 4 * 2); idx++)
            if (two_sect_buffer[idx] != 0xFF)
            {
                P_ERROR("[ERROR]: Test failed. Flash was not erased!\n");
                P_INFO("Exiting tests ...");
                os_free((void *)two_sect_buffer_space);
                return 1;
            }
    }
    P_INFO("[INFO]: Test 4 successfully completed\n");
    P_INFO("[INFO]: Test 5 - boundary checks\n");
    {
        res = esp_spiffs_read(FS_START - 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_read(FS_END + 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_read(FS_END - 4, 8, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_write(FS_START - 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_write(FS_END + 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_write(FS_END - 4, 8, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_erase(FS_START - 1, 1024 * 4 * 2);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_erase(FS_END + 1, 1024 * 4 * 2);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
        res = esp_spiffs_erase(FS_END - 4, 8);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return 1;
        }
    }
    P_INFO("[INFO]: Test 5 successfully completed\n");
    P_INFO("[INFO]: That's all for flash function testing.\n");
    os_free((void *)two_sect_buffer_space);
    P_DEBUG("[DEBUG]: Available heap size: %d\n", system_get_free_heap_size());
    return 0;
}

void ICACHE_FLASH_ATTR file_ls(Flashfs *t_fs)
{
    static struct spiffs_dirent ffile;
    struct spiffs_dirent *pfile;
    P_INFO("[INFO]: File system content:\n");
    pfile = t_fs->list(0);
    while (pfile)
    {
        P_INFO("        => %s [%X] size:%d\n", pfile->name, pfile->obj_id, pfile->size);
        pfile = t_fs->list(1);
    }
    P_INFO("[INFO]: End.\n");
}

// #pragma GCC diagnostic ignored "-Wwrite-strings"

Flashfs fs;

int ICACHE_FLASH_ATTR flashfs_class_test(void)
{
    P_INFO("[INFO]: File system tests start\n");

    P_INFO("[INFO]: Test 1 - check that before init the file system is unavailable\n");
    {
        Flashfs fs_not_init;
        P_TRACE("[TRACE]: Trying to format a not initialized FS\n");
        P_WARN("[WARNING]: Visually check the result!!\n");
        fs_not_init.format();
        P_TRACE("[TRACE]: Trying to unmount a not initialized FS\n");
        P_WARN("[WARNING]: Visually check the result!!\n");
        fs_not_init.unmount();
        P_TRACE("[TRACE]: Trying to check a not initialized FS\n");
        P_WARN("[WARNING]: Visually check the result!!\n");
        fs_not_init.check();
        P_TRACE("[TRACE]: Trying to get handler of a not initialized FS\n");
        spiffs *fs_handler = fs_not_init.get_handler();
        if (fs_handler)
        {
            P_TRACE("[TRACE]: Got a not NULL handler\n");
            P_TRACE("[TRACE]: Test failed\n");
        }
        struct spiffs_dirent *pfile = fs_not_init.list(0);
        if (pfile)
        {
            P_TRACE("[TRACE]: Got a not NULL pointer\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
        flashfs_status res = fs_not_init.get_status();
        if (res != FFS_NOT_INIT)
        {
            P_TRACE("[TRACE]: Got wrong status\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
        if (fs_not_init.is_available())
        {
            P_TRACE("[TRACE]: Got wrong status\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
        s32_t error = fs_not_init.last_error();
        if (error != 0)
        {
            P_TRACE("[TRACE]: Got wrong error\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
        u32_t size = fs_not_init.get_total_size();
        if (size != 0)
        {
            P_TRACE("[TRACE]: Got wrong size\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
        u32_t used_size = fs_not_init.get_used_size();
        if (used_size != 0)
        {
            P_TRACE("[TRACE]: Got wrong used_size\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
    }
    P_INFO("[INFO]: Test 1 completed\n");

    P_INFO("[INFO]: Test 2 - file system functions once the FS is mounted\n");
    {
        P_TRACE("[TRACE]: FS status: %d\n", fs.get_status());
        P_TRACE("[TRACE]: Mounting the FS\n");
        P_WARN("[WARNING]: Visually check the result!!\n");
        fs.init();
        P_TRACE("[TRACE]: FS status: %d\n", fs.get_status());
        if (fs.is_available())
        {
            P_INFO("[INFO]: Checking file system, this will take a while ...\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            fs.check();
            P_INFO("[INFO]: SPIFFS last error: %d\n", fs.last_error());
            P_INFO("[INFO]: Listing FS content\n");
            file_ls(&fs);
            P_WARN("[WARNING]: Visually check the result!!\n");
        }
    }
    P_INFO("[INFO]: Test 2 completed\n");

    P_INFO("[INFO]: Test 3 - file system unmount\n");
    {
        P_TRACE("[TRACE]: unounting the FS\n");
        P_WARN("[WARNING]: Visually check the result!!\n");
        fs.unmount();
        P_TRACE("[TRACE]: FS status: %d\n", fs.get_status());
        if (fs.is_available())
        {
            P_INFO("[INFO]: FS cannot be available after unounting\n");
            P_TRACE("[TRACE]: Test failed\n");
            return 1;
        }
    }
    P_INFO("[INFO]: Test 3 completed\n");

    P_INFO("[INFO]: File system test completed\n");
    return 0;
}

int ICACHE_FLASH_ATTR ffile_class_test(void)
{
    P_INFO("[INFO]: File tests start\n");

    P_INFO("[INFO]: Mounting the FS\n");
    fs.init();

    if (fs.is_available())
    {
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 1 - check that without a filename the file is unavailable\n");
            Ffile cfgfile(&fs);
            int res;
            char buffer[128];
            char *str = "test";
            if (cfgfile.is_available())
                P_TRACE("[TRACE]: the file is not available\n");
            else
            {
                P_TRACE("[TRACE]: the file is available\n");
                P_ERROR("[ERROR]: the test failed\n");
                return 1;
            }
            P_TRACE("[TRACE]: trying to read from file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            P_TRACE("[TRACE]: read returned %d\n", cfgfile.n_read(buffer, 2));

            P_TRACE("[TRACE]: trying to write to file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            P_TRACE("[TRACE]: write returned %d\n", cfgfile.n_append(str, 5));

            P_TRACE("[TRACE]: trying to clear file\n");

            P_WARN("[WARNING]: Visually check the result!!\n");
            cfgfile.clear();

            P_TRACE("[TRACE]: trying to remove file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            cfgfile.remove();

            P_TRACE("[TRACE]: trying to flush_cache file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            cfgfile.flush_cache();

            file_ls(&fs);
            P_INFO("[INFO]: Test 1 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 2 - write to file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs, "first_file.txt");
            char *str = "test";
            if (cfgfile.is_available())
            {
                P_TRACE("[TRACE]: write returned %d\n", cfgfile.n_append(str, 5));
            }
            file_ls(&fs);
            P_INFO("[INFO]: Test 2 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 3 - read from file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs, "first_file.txt");
            char buffer[128];
            os_memset(buffer, 'X', 128);
            if (cfgfile.is_available())
            {
                P_TRACE("[TRACE]: read returned %d\n", cfgfile.n_read(buffer, 128));
            }

            P_TRACE("[TRACE]: file content: ");
            for (int ii = 0; ii < 128; ii++)
            {
                if (buffer[ii])
                    P_TRACE("%c", buffer[ii]);
                else
                    P_TRACE("0");
            }
            P_TRACE("\n");

            file_ls(&fs);
            P_INFO("[INFO]: Test 3 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 4 - clear file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs, "first_file.txt");
            char buffer[128];
            os_memset(buffer, 'X', 128);
            P_TRACE("[TRACE]: clearing file, check the file size\n");
            if (cfgfile.is_available())
                cfgfile.clear();
            file_ls(&fs);

            P_TRACE("[TRACE]: now trying to read from file\n");
            if (cfgfile.is_available())
            {
                P_TRACE("[TRACE]: read returned %d\n", cfgfile.n_read(buffer, 128));
            }

            P_TRACE("[TRACE]: file content: ");
            for (int ii = 0; ii < 128; ii++)
            {
                if (buffer[ii])
                    P_TRACE("%c", buffer[ii]);
                else
                    P_TRACE("0");
            }
            P_TRACE("\n");

            P_INFO("[INFO]: Test 4 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 5 - flushing file cache\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs, "first_file.txt");
            char *str = "test";
            if (cfgfile.is_available())
            {
                P_TRACE("[TRACE]: write returned %d\n", cfgfile.n_append(str, 5));
                cfgfile.flush_cache();
            }
            file_ls(&fs);
            P_INFO("[INFO]: Test 5 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 6 - delete file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs, "first_file.txt");
            P_TRACE("[TRACE]: deleting file\n");
            if (cfgfile.is_available())
                cfgfile.remove();
            file_ls(&fs);

            P_INFO("[INFO]: Test 6 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 7 - creating a file with filename longer that 30 characters\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs, "aAaAaAaA10aAaAaAaA20aAaAaAaA30.txt");
            file_ls(&fs);
            P_INFO("[INFO]: Test 7 completed\n");
        }
        {
            file_ls(&fs);
            P_INFO("[INFO]: Test 8 - remove previously created file\n");
            P_WARN("[WARNING]: Visually check the result!!\n");
            Ffile cfgfile(&fs);
            if (cfgfile.is_available())
            {
                cfgfile.open("aAaAaAaA10aAaAaAaA20aAaAaAaA30");
                cfgfile.remove();
            }
            file_ls(&fs);
            P_INFO("[INFO]: Test 8 completed\n");
        }
    }
    P_INFO("[INFO]: File class tests completed\n");
    return 0;
}

void ICACHE_FLASH_ATTR run_tests(void)
{
    if (flash_function_test())
        return;
    if (flashfs_class_test())
        return;
    if (ffile_class_test())
        return;
}