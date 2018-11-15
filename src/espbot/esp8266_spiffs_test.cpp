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

class flashfs_test : public flashfs
{
  public:
    void flash_function_test(void);
};

void ICACHE_FLASH_ATTR flashfs_test::flash_function_test(void)
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
                return;
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
        res = esp_spiffs_write(FS_START + 21, 15, (u8_t *)str);
        res = esp_spiffs_read(FS_START, 1024 * 4 * 2, two_sect_buffer);
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
                return;
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
            return;
        }
        res = esp_spiffs_read(FS_END + 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_read(FS_END - 4, 8, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Reading after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_write(FS_START - 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_write(FS_END + 1, 1024 * 4 * 2, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_write(FS_END - 4, 8, two_sect_buffer);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Writing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_erase(FS_START - 1, 1024 * 4 * 2);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing before flash file system start!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_erase(FS_END + 1, 1024 * 4 * 2);
        if (res != SPIFFS_FLASH_BOUNDARY_ERROR)
        {
            P_ERROR("[ERROR]: Test failed!\n");
            P_ERROR("[ERROR]: Erasing after flash file system end!\n");
            P_INFO("Exiting tests ...");
            os_free((void *)two_sect_buffer_space);
            return;
        }
        res = esp_spiffs_erase(FS_END - 4, 8);
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
    P_INFO("[INFO]: That's all for flash function testing.\n");
    os_free((void *)two_sect_buffer_space);
    P_DEBUG("[DEBUG]: Available heap size: %d\n", system_get_free_heap_size());
}

//void ICACHE_FLASH_ATTR file_ls(flashfs *t_fs)
void ICACHE_FLASH_ATTR file_ls(spiffs *t_fs)
{
    static spiffs_DIR dd;
    static struct spiffs_dirent ffile;
    struct spiffs_dirent *pfile;
    P_INFO("[INFO]: File system content:\n");
    SPIFFS_opendir(t_fs, "/", &dd);
    pfile = SPIFFS_readdir(&dd, &ffile);
    while (pfile)
    {
        P_INFO("        => %s [%X] size:%d\n", pfile->name, pfile->obj_id, pfile->size);
        pfile = SPIFFS_readdir(&dd, &ffile);
    }
    SPIFFS_closedir(&dd);
    P_INFO("[INFO]: End.\n");
}
#pragma GCC diagnostic ignored "-Wwrite-strings"

flashfs fs;

void ICACHE_FLASH_ATTR run_tests(void)
{
    flashfs fs;
    //fs.flash_function_test();
    fs.init();
    // fs.format();
    // fs.init();

    if (fs.is_available())
    {
        P_INFO("[INFO]: Checking file system, this will take a while ...\n");
        fs.check();

        P_INFO("[INFO]: SPIFFS last error: %d\n", fs.last_error());
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Removing files\n");
            ffile cfg_file(fs.get_handle(), (char *)"sixth_file.txt");
            cfg_file.remove();
            cfg_file.set_name("first_file.txt");
            cfg_file.remove();
            cfg_file.set_name("012345678901234567890123456789.txt");
            cfg_file.remove();
            cfg_file.set_name("third_file.txt");
            cfg_file.remove();
            cfg_file.set_name("fourth_file.txt");
            cfg_file.remove();
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Writing file 1\n");
            ffile cfg_file(fs.get_handle(), "first_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_append("content of the first file\n", 26);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_append("some more content\n", 18);
            cfg_file.n_append("and again some more ...\n", 24);
            P_INFO("[INFO]: File 1 written\n");
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Writing file 2\n");
            ffile cfg_file(fs.get_handle(), (char *)"012345678901234567890123456789.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_append((char *)"content of the second file", 26);
            cfg_file.n_append("some more content\n", 18);
            cfg_file.n_append("and again some more ...\n", 24);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            P_INFO("[INFO]: File 2 written\n");
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Writing file 3\n");
            ffile cfg_file(fs.get_handle(), (char *)"third_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_append((char *)"content of the third file", 25);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            P_INFO("[INFO]: File 3 written\n");
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Writing file 4\n");
            ffile cfg_file(fs.get_handle(), (char *)"fourth_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_append((char *)"content of the fourth file", 26);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            P_INFO("[INFO]: File 4 written\n");
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Writing file 5\n");
            ffile cfg_file(fs.get_handle(), (char *)"fifth_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_append((char *)"content of the fifth file", 25);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            P_INFO("[INFO]: File 5 written\n");
        }
        file_ls(fs.get_handle());
        {
            int res;
            P_INFO("[INFO]: Reading file 1\n");
            char *buffer = (char *)os_zalloc(256);
            ffile cfg_file(fs.get_handle(), (char *)"first_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            while (res = cfg_file.n_read(buffer, 255) > 0)
                P_INFO("File content: [%d] %s \n", res, buffer);
            P_TRACE("[TRACE]: last read result was: %d\n", res);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            os_free(buffer);
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Reading file 3\n");
            char *buffer = (char *)os_zalloc(256);
            ffile cfg_file(fs.get_handle(), (char *)"third_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.n_read(buffer, 255);
            P_INFO("File content: %s \n", buffer);
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            os_free(buffer);
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Removing file 5\n");
            ffile cfg_file(fs.get_handle(), (char *)"fifth_file.txt");
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            cfg_file.remove();
            P_TRACE("[TRACE]: File status: %d\n", cfg_file.get_status());
            P_INFO("[INFO]: File 5 removed\n");
        }
        file_ls(fs.get_handle());
        {
            P_INFO("[INFO]: Removing files\n");
            ffile cfg_file(fs.get_handle(), (char *)"sixth_file.txt");
            cfg_file.remove();
            cfg_file.set_name("first_file.txt");
            cfg_file.remove();
            cfg_file.set_name("012345678901234567890123456789");
            cfg_file.remove();
            cfg_file.set_name("third_file.txt");
            cfg_file.remove();
            cfg_file.set_name("fourth_file.txt");
            cfg_file.remove();
        }
        fs.check();
    }
}

void ICACHE_FLASH_ATTR run_tests_c(void)
{
    spiffs m_fs;
    u8_t m_work[LOG_PAGE_SIZE * 2]; //  a ram memory buffer being double the size of the logical page size
    u8_t m_fd_space[32 * 4];        // 4 file descriptors => 4 file opened simultaneously
                                    // NO CACHE. To enable cache <#define SPIFFS_CACHE 1> and uncomment following line for buffer definition
#if SPIFFS_CACHE
    u8_t m_cache[(LOG_PAGE_SIZE + 32) * 4]; // 1152 bytes => cache
#else
    u8_t m_cache[1];
#endif
    s32_t res;
    spiffs_config m_config;
    m_config.phys_size = FS_END - FS_START; // use all spi flash
    m_config.phys_addr = FS_START;          // start spiffs at start of spi flash
    m_config.phys_erase_block = (1024 * 4); // according to datasheet
    m_config.log_block_size = (1024 * 4);   // let us not complicate things
    m_config.log_page_size = LOG_PAGE_SIZE; // as we said
    m_config.hal_read_f = esp_spiffs_read;
    m_config.hal_write_f = esp_spiffs_write;
    m_config.hal_erase_f = esp_spiffs_erase;
    res = SPIFFS_mount(&m_fs,
                       &m_config,
                       m_work,
                       m_fd_space,
                       sizeof(m_fd_space),
                       m_cache,
                       sizeof(m_cache),
                       0);
    if (res != SPIFFS_OK)
    {
        if (res == SPIFFS_ERR_MAGIC_NOT_POSSIBLE)
        {
            P_FATAL("[FATAL]: Error mounting the file system, error code %d\n", res);
            P_FATAL("[FATAL]: Try another page size or block size.\n");
            return;
        }
        if (res == SPIFFS_ERR_NOT_A_FS)
        {
            P_TRACE("[TRACE]: Error mounting the file system, error code %d\n", res);
            P_TRACE("[TRACE]: Will try to format the file system.\n");
        }
        if (res != SPIFFS_ERR_NOT_A_FS)
        {
            P_TRACE("[TRACE]: Unmounting the file system.\n");
            SPIFFS_unmount(&m_fs);
        }
        P_TRACE("[TRACE]: Formatting the file system.\n");
        res = SPIFFS_format(&m_fs);
        if (res == SPIFFS_OK)
        {
            P_TRACE("[TRACE]: File system formatted.\n");
        }
        else
        {
            P_FATAL("[FATAL]: Cannot format file system, error code %d\n", res);
            return;
        }
        res = SPIFFS_mount(&m_fs,
                           &m_config,
                           m_work,
                           m_fd_space,
                           sizeof(m_fd_space),
                           m_cache,
                           sizeof(m_cache),
                           0);
        if (res != SPIFFS_OK)
        {
            P_FATAL("[FATAL]: Cannot mount file system, error code %d\n", res);
            return;
        }
    }
    P_TRACE("[TRACE]: File system mounted.\n");
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    P_INFO("[INFO]: File system size [bytes]: %d, used [bytes]:%d.\n", total, used);

    spiffs_file m_fd;
    file_ls(&m_fs);
    {
        char *m_name = "first_one.txt";
        m_fd = SPIFFS_open(&m_fs, m_name, SPIFFS_CREAT | SPIFFS_O_APPEND | SPIFFS_O_RDWR, 0);
        if (m_fd < 0)
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"content of the first file", 26);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"\n", 2);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"some more content", 18);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);

        res = SPIFFS_close(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_fflush(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(&m_fs), m_name);
    }
    file_ls(&m_fs);
    {
        char *m_name = "second_one.txt";
        m_fd = SPIFFS_open(&m_fs, m_name, SPIFFS_CREAT | SPIFFS_O_APPEND | SPIFFS_O_RDWR, 0);
        if (m_fd < 0)
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"content of the second file", 26);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"\n", 2);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"some more content", 18);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);

        res = SPIFFS_close(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_fflush(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(&m_fs), m_name);
    }
    file_ls(&m_fs);
    {
        char *m_name = "third_one.txt";
        m_fd = SPIFFS_open(&m_fs, m_name, SPIFFS_CREAT | SPIFFS_O_APPEND | SPIFFS_O_RDWR, 0);
        if (m_fd < 0)
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"content of the third file", 26);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"\n", 2);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"some more content", 18);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);

        res = SPIFFS_close(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_fflush(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(&m_fs), m_name);
    }
    file_ls(&m_fs);
    {
        char *m_name = "fourth_one.txt";
        m_fd = SPIFFS_open(&m_fs, m_name, SPIFFS_CREAT | SPIFFS_O_APPEND | SPIFFS_O_RDWR, 0);
        if (m_fd < 0)
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"content of the fourth file", 26);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"\n", 2);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_write(&m_fs, m_fd, (void *)"some more content", 18);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(&m_fs), m_name);

        res = SPIFFS_close(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_fflush(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(&m_fs), m_name);
    }
    file_ls(&m_fs);
    {
        char *m_name = "first_one.txt";
        m_fd = SPIFFS_open(&m_fs, m_name, SPIFFS_CREAT | SPIFFS_O_APPEND | SPIFFS_O_RDWR, 0);
        if (m_fd < 0)
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(&m_fs), m_name);
        char *buffer = (char *)os_zalloc(256);
        m_fd = SPIFFS_read(&m_fs, m_fd, buffer, 256);
        if (m_fd < 0)
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(&m_fs), m_name);

        P_INFO("File content [%s]: %s \n", m_name, buffer);

        res = SPIFFS_close(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(&m_fs), m_name);
        res = SPIFFS_fflush(&m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(&m_fs), m_name);
        os_free(buffer);
    }
}