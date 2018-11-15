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
#include "c_types.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "spi_flash.h"
}

#include "esp8266_spiffs.hpp"

// flash read function (checkout SPIFFS documentation)
s32_t ICACHE_FLASH_ATTR esp_spiffs_read(u32_t t_addr, u32_t t_size, u8_t *t_dst)
{
    // P_TRACE("[TRACE]: spiffs read called --------------------------------------\n");
    SpiFlashOpResult res;
    // find aligned start address
    u32_t start_addr = (t_addr / FS_ALIGN_BYTES) * FS_ALIGN_BYTES;
    // and how many bytes are required by alignment
    int align_bytes = t_addr % FS_ALIGN_BYTES;

    // boundary checks
    if ((start_addr < FS_START) || (start_addr >= FS_END) ||
        (start_addr + ((t_size / FS_ALIGN_BYTES) * FS_ALIGN_BYTES) > FS_END))
    {
        P_ERROR("[ERROR]: Flash file system boundary error!\n");
        P_ERROR("[ERROR]: Reading from address: %X, size: %d\n", t_addr, t_size);
        return SPIFFS_FLASH_BOUNDARY_ERROR;
    }

    // let's use aligned ram variables
    // warning: using stack instead of heap will produce hallucinations
    uint32 buffer_space = (uint32)os_malloc(LOG_PAGE_SIZE + FS_ALIGN_BYTES);
    uint32 *buffer = (uint32 *)(((buffer_space + FS_ALIGN_BYTES) / FS_ALIGN_BYTES) * FS_ALIGN_BYTES);

    while (t_size > 0)
    {
        // P_TRACE("[TRACE]: bytes to be read %d, unaligned addr %X, aligned addr %X, align bytes %d\n",
        //         t_size, t_addr, start_addr, align_bytes);
        // read F_BUF_SIZE bytes from flash
        res = spi_flash_read(start_addr, buffer, LOG_PAGE_SIZE);
        system_soft_wdt_feed();
        if (res == SPI_FLASH_RESULT_ERR)
        {
            P_ERROR("[ERROR]: error reading flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
            os_free((void *)buffer_space);
            return SPIFFS_FLASH_RESULT_ERR;
        }
        if (res == SPI_FLASH_RESULT_TIMEOUT)
        {
            P_ERROR("[ERROR]: timeout reading flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
            os_free((void *)buffer_space);
            return SPIFFS_FLASH_RESULT_TIMEOUT;
        }

        // check if must read again from flash
        if (t_size > (LOG_PAGE_SIZE - align_bytes))
        {
            // discard initial bytes required by alignment (if any)
            // and copy to destination
            // P_TRACE("[TRACE]: copying %d bytes from %X to bytes to %X\n",
            //         (LOG_PAGE_SIZE - align_bytes), buffer + align_bytes, t_dst);
            os_memcpy(t_dst, buffer + align_bytes, (LOG_PAGE_SIZE - align_bytes));
            t_dst += (LOG_PAGE_SIZE - align_bytes);
            t_size -= (LOG_PAGE_SIZE - align_bytes);
            start_addr += (LOG_PAGE_SIZE);
            align_bytes = 0;
        }
        else
        {
            // just copy required bytes
            // P_TRACE("[TRACE]: copying %d bytes from %X to bytes to %X\n",
            //         t_size, buffer + align_bytes, t_dst);
            os_memcpy(t_dst, buffer + align_bytes, t_size);
            t_size = 0;
        }
    }
    os_free((void *)buffer_space);
    return SPIFFS_OK;
}

// flash write function (checkout SPIFFS documentation)
s32_t ICACHE_FLASH_ATTR esp_spiffs_write(u32_t t_addr, u32_t t_size, u8_t *t_src)
{
    // P_TRACE("[TRACE]: spiffs write called -------------------------------------\n");
    SpiFlashOpResult res;
    // find aligned start address
    u32_t start_addr = (t_addr / FS_ALIGN_BYTES) * FS_ALIGN_BYTES;
    // and how many bytes are required by alignment
    u8_t align_bytes = t_addr % FS_ALIGN_BYTES;

    // boundary checks
    if ((start_addr < FS_START) || (start_addr >= FS_END) ||
        (start_addr + ((t_size / FS_ALIGN_BYTES) * FS_ALIGN_BYTES) > FS_END))
    {
        P_ERROR("[ERROR]: Flash file system boundary error!\n");
        P_ERROR("[ERROR]: Writing to address: %X, size: %d\n", t_addr, t_size);
        return SPIFFS_FLASH_BOUNDARY_ERROR;
    }

    // let's use aligned ram variable
    // warning: using stack instead of heap will produce hallucinations
    uint32 buffer_space = (uint32)os_malloc(LOG_PAGE_SIZE + FS_ALIGN_BYTES);
    uint32 *buffer = (uint32 *)(((buffer_space + FS_ALIGN_BYTES) / FS_ALIGN_BYTES) * FS_ALIGN_BYTES);

    while (t_size > 0)
    {
        // P_TRACE("[TRACE]: bytes to be written %d, unaligned addr %X, aligned addr %X, align bytes %d\n",
        //         t_size, t_addr, start_addr, align_bytes);
        // read LOG_PAGE_SIZE bytes from flash into buffer
        res = spi_flash_read(start_addr, (uint32 *)buffer, LOG_PAGE_SIZE);
        system_soft_wdt_feed();
        if (res == SPI_FLASH_RESULT_ERR)
        {
            P_ERROR("[ERROR]: error reading flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
            os_free((void *)buffer_space);
            return SPIFFS_FLASH_RESULT_ERR;
        }
        if (res == SPI_FLASH_RESULT_TIMEOUT)
        {
            P_ERROR("[ERROR]: timeout reading flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
            os_free((void *)buffer_space);
            return SPIFFS_FLASH_RESULT_TIMEOUT;
        }

        if (t_size > (LOG_PAGE_SIZE - align_bytes))
        {
            // discard initial bytes required by alignment (if any)
            // and copy source data to buffer
            // P_TRACE("[TRACE]: copying %d bytes from %X to bytes to %X\n",
            //         LOG_PAGE_SIZE - align_bytes, t_src, (u8_t *)buffer + align_bytes);
            os_memcpy((u8_t *)buffer + align_bytes, t_src, LOG_PAGE_SIZE - align_bytes);
            // and write buffer to flash
            // P_TRACE("[TRACE]: writing %d bytes to flash %X from %X\n",
            //         LOG_PAGE_SIZE, start_addr, buffer);
            res = spi_flash_write(start_addr, (uint32 *)buffer, LOG_PAGE_SIZE);
            system_soft_wdt_feed();

            if (res == SPI_FLASH_RESULT_ERR)
            {
                P_ERROR("[ERROR]: Error writing flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
                os_free((void *)buffer_space);
                return SPIFFS_FLASH_RESULT_ERR;
            }
            if (res == SPI_FLASH_RESULT_TIMEOUT)
            {
                P_ERROR("[ERROR]: Timeout writing flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
                os_free((void *)buffer_space);
                return SPIFFS_FLASH_RESULT_TIMEOUT;
            }

            t_src += (LOG_PAGE_SIZE - align_bytes);
            t_size -= (LOG_PAGE_SIZE - align_bytes);
            start_addr += (LOG_PAGE_SIZE);
            align_bytes = 0;
        }
        else
        {
            // just copy required bytes to buffer
            // P_TRACE("[TRACE]: copying %d bytes from %X to bytes to %X\n",
            //         t_size, t_src, (u8_t *)buffer + align_bytes);
            os_memcpy((u8_t *)buffer + align_bytes, t_src, t_size);
            // and write buffer to flash
            // P_TRACE("[TRACE]: writing %d bytes to flash %X from %X\n",
            //         LOG_PAGE_SIZE, start_addr, buffer);
            res = spi_flash_write(start_addr, (uint32 *)buffer, LOG_PAGE_SIZE);
            system_soft_wdt_feed();

            if (res == SPI_FLASH_RESULT_ERR)
            {
                P_ERROR("[ERROR]: Error writing flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
                os_free((void *)buffer_space);
                return SPIFFS_FLASH_RESULT_ERR;
            }
            if (res == SPI_FLASH_RESULT_TIMEOUT)
            {
                P_ERROR("[ERROR]: Timeout writing flash from %X for %d bytes\n", start_addr, LOG_PAGE_SIZE);
                os_free((void *)buffer_space);
                return SPIFFS_FLASH_RESULT_TIMEOUT;
            }
            t_size = 0;
        }
    }
    os_free((void *)buffer_space);
    return SPIFFS_OK;
}

// flash erase function (checkout SPIFFS documentation)
s32_t ICACHE_FLASH_ATTR esp_spiffs_erase(u32_t t_addr, u32_t t_size)
{
    // P_TRACE("[TRACE]: spiffs erase called ------------------------------------\n");
    SpiFlashOpResult res;
    // boundary checks
    if ((((t_addr / FS_ALIGN_BYTES) * FS_ALIGN_BYTES) < FS_START) ||
        (((t_addr / FS_ALIGN_BYTES) * FS_ALIGN_BYTES) >= FS_END) ||
        (((t_addr / FS_ALIGN_BYTES) * FS_ALIGN_BYTES) + ((t_size / FS_ALIGN_BYTES) * FS_ALIGN_BYTES) > FS_END))
    {
        P_ERROR("[ERROR]: Flash file system boundary error!\n");
        P_ERROR("[ERROR]: Erasing from address: %X, size: %d\n", t_addr, t_size);
        return SPIFFS_FLASH_BOUNDARY_ERROR;
    }

    // find sector number and offset from sector start
    uint16_t sect_number = t_addr / FLASH_SECT_SIZE;
    uint32_t sect_offset = t_addr % FLASH_SECT_SIZE;

    while (t_size > 0)
    {
        // P_TRACE("[TRACE]: bytes to be erased %d, sector num %d, sector offset %d\n",
        //         t_size, sect_number, sect_offset);
        // erase sector
        res = spi_flash_erase_sector(sect_number);
        if (res == SPI_FLASH_RESULT_ERR)
        {
            P_ERROR("[ERROR]: Error erasing flash sector %d\n", sect_number);
            return SPIFFS_FLASH_RESULT_ERR;
        }
        if (res == SPI_FLASH_RESULT_TIMEOUT)
        {
            P_ERROR("[ERROR]: Timeout erasing flash sector %d\n", sect_number);
            return SPIFFS_FLASH_RESULT_TIMEOUT;
        }

        // check if must delete more than one sector
        if (t_size > (FLASH_SECT_SIZE - sect_offset))
        {
            t_size -= (FLASH_SECT_SIZE - sect_offset);
            sect_number += 1;
            sect_offset = 0;
        }
        else
        {
            t_size = 0;
        }
    }
    return SPIFFS_OK;
}

void ICACHE_FLASH_ATTR flashfs::init(void)
{
    // SPIFFS_USE_MAGIC is enabled so following documentation:
    // 1) Call SPIFFS_mount
    // 2) If SPIFFS_mount fails with SPIFFS_ERR_NOT_A_FS, keep going.
    // 3) Otherwise, call SPIFFS_unmount and call SPIFFS_format
    // 4) Call SPIFFS_mount again.
    s32_t res;

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
            status = FFS_UNAVAILABLE;
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
            status = FFS_UNAVAILABLE;
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
            status = FFS_UNAVAILABLE;
            return;
        }
    }
    P_TRACE("[TRACE]: File system mounted.\n");
    status = FFS_AVAILABLE;
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    P_INFO("[INFO]: File system size [bytes]: %d, used [bytes]:%d.\n", total, used);
}

void ICACHE_FLASH_ATTR flashfs::format(void)
{
    s32_t res;
    P_TRACE("[TRACE]: Unmounting the file system.\n");
    SPIFFS_unmount(&m_fs);
    P_TRACE("[TRACE]: Formatting the file system.\n");
    res = SPIFFS_format(&m_fs);
    if (res == SPIFFS_OK)
    {
        P_TRACE("[TRACE]: File system formatted.\n");
        status = FFS_UNMOUNTED;
    }
    else
    {
        P_FATAL("[FATAL]: Cannot format file system, error code %d\n", res);
        status = FFS_UNAVAILABLE;
    }
}

flashfs_status ICACHE_FLASH_ATTR flashfs::get_status()
{
    return status;
}

bool ICACHE_FLASH_ATTR flashfs::is_available()
{
    if ((status == FFS_AVAILABLE) || (FFS_CHECK_ERRORS))
        return true;
    else
        return false;
}

s32_t ICACHE_FLASH_ATTR flashfs::last_error()
{
    return SPIFFS_errno(&m_fs);
}

u32_t ICACHE_FLASH_ATTR flashfs::get_total_size()
{
    s32_t res;
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    return total;
}
u32_t ICACHE_FLASH_ATTR flashfs::get_used_size()
{
    s32_t res;
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    return used;
}

s32_t ICACHE_FLASH_ATTR flashfs::check()
{
    s32_t res = SPIFFS_check(&m_fs);
    if (res == SPIFFS_OK)
    {
        P_TRACE("[TRACE]: Successfully checked the file system.\n");
        status = FFS_AVAILABLE;
    }
    else
    {
        P_ERROR("[ERROR]: File system check found errors, error code %d\n", res);
        status = FFS_CHECK_ERRORS;
    }
    return res;
}

struct spiffs_dirent ICACHE_FLASH_ATTR *flashfs::list(int t_file)
{
    static spiffs_DIR dd;
    static struct spiffs_dirent ffile;
    struct spiffs_dirent *pfile;

    if (t_file == 0)
        SPIFFS_opendir(&m_fs, "/", &dd);
    pfile = SPIFFS_readdir(&dd, &ffile);
    if (pfile == NULL)
        SPIFFS_closedir(&dd);
    return pfile;
}

spiffs ICACHE_FLASH_ATTR *flashfs::get_handle()
{
    return &m_fs;
}

// create a new file with no name, no operations will be permitted
// the file status is set to FFS_F_UNAVAILABLE
ICACHE_FLASH_ATTR ffile::ffile(spiffs *t_fs)
{
    m_fs = t_fs;
    m_name[0] = '\0';
    status = FFS_F_UNAVAILABLE;
}

// create a new file variable with the specified name
// create a new file, or open if it exists, ready for READ and WRITE (APPEND) operations
// in case of errors the file status is set to FFS_F_UNAVAILABLE
ICACHE_FLASH_ATTR ffile::ffile(spiffs *t_fs, char *t_filename)
{
    m_fs = t_fs;
    os_strncpy(m_name, t_filename, 30);
    if (os_strlen(t_filename) > 30)
    {
        P_WARN("[WARNING]: Filename will be truncated to 30 characters\n");
        m_name[30] = '\0';
    }
    m_fd = SPIFFS_open(m_fs, m_name, SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_APPEND, 0);
    if (m_fd < 0)
    {
        P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(m_fs), m_name);
        status = FFS_F_UNAVAILABLE;
    }
    else
        status = FFS_F_OPEN;
}

// close the file (if open)
// and eventually flush chache to flash memory
ICACHE_FLASH_ATTR ffile::~ffile()
{
    if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
    {
        s32_t res = SPIFFS_close(m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(m_fs), m_name);
    }
}

// return the file name
char ICACHE_FLASH_ATTR *ffile::get_name()
{
    if (os_strlen(m_name) == 0)
    {
        P_ERROR("[ERROR]: The file has no name\n");
        status = FFS_F_UNAVAILABLE;
    }
    return m_name;
}

// set the filename
// if the file was open with a different filename it will be closed and changed saved to flash
// then
// create a new file, or open if it exists, ready for READ and WRITE (APPEND) operations
// in case of errors the file status is set to FFS_F_UNAVAILABLE
void ICACHE_FLASH_ATTR ffile::set_name(char *t_filename)
{
    if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
    {
        s32_t res = SPIFFS_close(m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(m_fs), m_name);
    }
    os_strncpy(m_name, t_filename, 30);
    if (os_strlen(t_filename) > 30)
    {
        P_WARN("[WARNING]: Filename will be truncated to 30 characters\n");
        m_name[30] = '\0';
    }
    m_fd = SPIFFS_open(m_fs, m_name, SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_APPEND, 0);
    if (m_fd < 0)
    {
        P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(m_fs), m_name);
        status = FFS_F_UNAVAILABLE;
    }
    else
        status = FFS_F_OPEN;
}

// return the file status
flashfs_file_status ICACHE_FLASH_ATTR ffile::get_status()
{
    return status;
}

// read t_len bytes from the file to the t_buffer
int ICACHE_FLASH_ATTR ffile::n_read(char *t_buffer, int t_len)
{
    s32_t res = 0;
    if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
    {
        res = SPIFFS_read(m_fs, m_fd, (u8_t *)t_buffer, t_len);
        if (res < SPIFFS_OK)
        {
            P_ERROR("[ERROR]: Error %d while reading from file %s\n", SPIFFS_errno(m_fs), m_name);
        }
    }
    else
    {
        P_ERROR("[ERROR]: Cannot read from file %s, file status is %d\n", m_name, status);
    }
    return (int)res;
}

// write (append) t_len bytes from the t_buffer to the file
int ICACHE_FLASH_ATTR ffile::n_append(char *t_buffer, int t_len)
{
    s32_t res = 0;
    if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
    {
        res = SPIFFS_write(m_fs, m_fd, (u8_t *)t_buffer, t_len);
        if (res < SPIFFS_OK)
        {
            P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(m_fs), m_name);
        }
        else
            status = FFS_F_MODIFIED_UNSAVED;
    }
    else
    {
        P_ERROR("[ERROR]: Cannot write to file %s, file status is %d\n", m_name, status);
    }
    return (int)res;
}

// clear the file content
void ICACHE_FLASH_ATTR ffile::clear()
{
    if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
    {
        s32_t res = SPIFFS_close(m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(m_fs), m_name);
        m_fd = SPIFFS_open(m_fs, m_name, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR | SPIFFS_APPEND, 0);
        if (m_fd < 0)
        {
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(m_fs), m_name);
            status = FFS_F_UNAVAILABLE;
        }
        else
            status = FFS_F_MODIFIED_UNSAVED;
    }
}

// remove the file
void ICACHE_FLASH_ATTR ffile::remove()
{
    if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
    {
        s32_t res = SPIFFS_fremove(m_fs, m_fd);
        if (res != SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while removing file %s\n", SPIFFS_errno(m_fs), m_name);
        else
            status = FFS_F_REMOVED;
    }
}

// flush chached changes to the flash memory
void ICACHE_FLASH_ATTR ffile::flush_cache()
{
    if (status == FFS_F_MODIFIED_UNSAVED)
    {
        s32_t res = SPIFFS_fflush(m_fs, m_fd);
        if (res < SPIFFS_OK)
            P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(m_fs), m_name);
    }
}
