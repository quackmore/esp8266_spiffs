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
#include "spiffs_flash_functions.h"
}

#include "esp8266_spiffs.hpp"

ICACHE_FLASH_ATTR Flashfs::Flashfs()
{
    status = FFS_NOT_INIT;
}

void ICACHE_FLASH_ATTR Flashfs::init(void)
{
    // SPIFFS_USE_MAGIC is enabled so following documentation:
    // 1) Call SPIFFS_mount
    // 2) If SPIFFS_mount fails with SPIFFS_ERR_NOT_A_FS, keep going.
    // 3) Otherwise, call SPIFFS_unmount and call SPIFFS_format
    // 4) Call SPIFFS_mount again.
    s32_t res;
    status = FFS_UNAVAILABLE;

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
    P_INFO("[INFO]: File system mounted.\n");
    status = FFS_AVAILABLE;
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    P_INFO("[INFO]: File system size [bytes]: %d, used [bytes]:%d.\n", total, used);
}

void ICACHE_FLASH_ATTR Flashfs::format(void)
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: formatting a not initialized file system\n");
        return;
    }
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

void ICACHE_FLASH_ATTR Flashfs::unmount(void)
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: unmounting a not initialized file system\n");
        return;
    }
    SPIFFS_unmount(&m_fs);
    P_TRACE("[TRACE]: File system unmounted.\n");
    status = FFS_UNMOUNTED;
}

flashfs_status ICACHE_FLASH_ATTR Flashfs::get_status()
{
    return status;
}

bool ICACHE_FLASH_ATTR Flashfs::is_available()
{
    if ((status == FFS_AVAILABLE) || (status == FFS_ERRORS))
        return true;
    else
        return false;
}

s32_t ICACHE_FLASH_ATTR Flashfs::last_error()
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: looking for last error of a not initialized file system\n");
        return 0;
    }
    return SPIFFS_errno(&m_fs);
}

u32_t ICACHE_FLASH_ATTR Flashfs::get_total_size()
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: looking for total size of a not initialized file system\n");
        return 0;
    }
    s32_t res;
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    return total;
}
u32_t ICACHE_FLASH_ATTR Flashfs::get_used_size()
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: looking for used size of a not initialized file system\n");
        return 0;
    }
    s32_t res;
    u32_t total = 0;
    u32_t used = 0;
    res = SPIFFS_info(&m_fs, &total, &used);
    return used;
}

s32_t ICACHE_FLASH_ATTR Flashfs::check()
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: checking a not initialized file system\n");
        return 0;
    }
    s32_t res = SPIFFS_check(&m_fs);
    if (res == SPIFFS_OK)
    {
        P_TRACE("[TRACE]: Successfully checked the file system.\n");
        status = FFS_AVAILABLE;
    }
    else
    {
        P_ERROR("[ERROR]: File system check found errors, error code %d\n", res);
        status = FFS_ERRORS;
    }
    return res;
}

struct spiffs_dirent ICACHE_FLASH_ATTR *Flashfs::list(int t_file)
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: listing a not initialized file system\n");
        return NULL;
    }
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

spiffs ICACHE_FLASH_ATTR *Flashfs::get_handler()
{
    if (status == FFS_NOT_INIT)
    {
        P_ERROR("[ERROR]: looking for handler of a not initialized file system\n");
        return NULL;
    }
    return &m_fs;
}

// create a new file with no name, no operations will be permitted
// the file status is set to FFS_F_UNAVAILABLE
ICACHE_FLASH_ATTR Ffile::Ffile(Flashfs *t_fs)
{
    status = FFS_F_UNAVAILABLE;
    m_name[0] = '\0';
    if (t_fs->is_available())
    {
        m_fs = t_fs;
    }
    else
    {
        m_fs = NULL;
        P_ERROR("[ERROR]: creating file on a not available file system\n");
    }
}

// create a new file variable with the specified name
// create a new file, or open if it exists, ready for READ and WRITE (APPEND) operations
// in case of errors the file status is set to FFS_F_UNAVAILABLE
ICACHE_FLASH_ATTR Ffile::Ffile(Flashfs *t_fs, char *t_filename)
{
    status = FFS_F_UNAVAILABLE;
    os_strncpy(m_name, t_filename, 30);
    if (os_strlen(t_filename) > 30)
    {
        P_WARN("[WARNING]: Filename will be truncated to 30 characters\n");
        m_name[30] = '\0';
    }
    if (t_fs->is_available())
    {
        m_fs = t_fs;
        m_fd = SPIFFS_open(m_fs->get_handler(), m_name, SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_APPEND, 0);
        if (m_fd < 0)
        {
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
        }
        else
            status = FFS_F_OPEN;
    }
    else
    {
        m_fs = NULL;
        P_ERROR("[ERROR]: creating file on a not available file system\n");
    }
}

// close the file (if open)
// and eventually flush chache to flash memory
ICACHE_FLASH_ATTR Ffile::~Ffile()
{
    if (m_fs && (m_fs->is_available()))
    {
        if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
        {
            s32_t res = SPIFFS_close(m_fs->get_handler(), m_fd);
            if (res != SPIFFS_OK)
                P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
        }
    }
}

// return the file name
char ICACHE_FLASH_ATTR *Ffile::get_name()
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
void ICACHE_FLASH_ATTR Ffile::open(char *t_filename)
{
    if (m_fs && (m_fs->is_available()))
    {
        if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
        {
            s32_t res = SPIFFS_close(m_fs->get_handler(), m_fd);
            if (res != SPIFFS_OK)
                P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
        }
        os_strncpy(m_name, t_filename, 30);
        if (os_strlen(t_filename) > 30)
        {
            P_WARN("[WARNING]: Filename will be truncated to 30 characters\n");
            m_name[30] = '\0';
        }
        m_fd = SPIFFS_open(m_fs->get_handler(), m_name, SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_APPEND, 0);
        if (m_fd < 0)
        {
            P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
            status = FFS_F_UNAVAILABLE;
        }
        else
            status = FFS_F_OPEN;
    }
    else
    {
        status = FFS_F_UNAVAILABLE;
        P_ERROR("[ERROR]: opening file on a not available file system\n");
    }
}

// return the file status
flashfs_file_status ICACHE_FLASH_ATTR Ffile::get_status()
{
    return status;
}

// return the file status
bool ICACHE_FLASH_ATTR Ffile::is_available()
{
    if ((status == FFS_F_OPEN) || (FFS_F_MODIFIED_UNSAVED))
        return true;
    else
        return false;
}

// read t_len bytes from the file to the t_buffer
int ICACHE_FLASH_ATTR Ffile::n_read(char *t_buffer, int t_len)
{
    s32_t res = 0;
    if (m_fs && (m_fs->is_available()))
    {
        if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
        {
            res = SPIFFS_read(m_fs->get_handler(), m_fd, (u8_t *)t_buffer, t_len);
            if (res < SPIFFS_OK)
            {
                P_ERROR("[ERROR]: Error %d while reading from file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
            }
        }
        else
        {
            P_ERROR("[ERROR]: Cannot read from file %s, file status is %d\n", m_name, status);
        }
    }
    else
    {
        status = FFS_F_UNAVAILABLE;
        P_ERROR("[ERROR]: reading file on a not available file system\n");
        res = -1;
    }
    return (int)res;
}

// write (append) t_len bytes from the t_buffer to the file
int ICACHE_FLASH_ATTR Ffile::n_append(char *t_buffer, int t_len)
{
    s32_t res = 0;
    if (m_fs && (m_fs->is_available()))
    {
        if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
        {
            res = SPIFFS_write(m_fs->get_handler(), m_fd, (u8_t *)t_buffer, t_len);
            if (res < SPIFFS_OK)
            {
                P_ERROR("[ERROR]: Error %d while writing to file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
            }
            else
                status = FFS_F_MODIFIED_UNSAVED;
        }
        else
        {
            P_ERROR("[ERROR]: Cannot write to file %s, file status is %d\n", m_name, status);
        }
    }
    else
    {
        status = FFS_F_UNAVAILABLE;
        P_ERROR("[ERROR]: writing file on a not available file system\n");
        res = -1;
    }
    return (int)res;
}

// clear the file content
void ICACHE_FLASH_ATTR Ffile::clear()
{
    if (m_fs && (m_fs->is_available()))
    {
        if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
        {
            s32_t res = SPIFFS_close(m_fs->get_handler(), m_fd);
            if (res != SPIFFS_OK)
                P_ERROR("[ERROR]: Error %d while closing file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
            m_fd = SPIFFS_open(m_fs->get_handler(), m_name, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR | SPIFFS_APPEND, 0);
            if (m_fd < 0)
            {
                P_ERROR("[ERROR]: Error %d while opening file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
                status = FFS_F_UNAVAILABLE;
            }
            else
                status = FFS_F_MODIFIED_UNSAVED;
        }
        else
        {
            P_ERROR("[ERROR]: Cannot clear file %s, file status is %d\n", m_name, status);
        }
    }
    else
    {
        status = FFS_F_UNAVAILABLE;
        P_ERROR("[ERROR]: clearing file on a not available file system\n");
    }
}

// remove the file
void ICACHE_FLASH_ATTR Ffile::remove()
{
    if (m_fs && (m_fs->is_available()))
    {
        if ((status == FFS_F_OPEN) || (status == FFS_F_MODIFIED_UNSAVED))
        {
            s32_t res = SPIFFS_fremove(m_fs->get_handler(), m_fd);
            if (res != SPIFFS_OK)
                P_ERROR("[ERROR]: Error %d while removing file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
            else
                status = FFS_F_REMOVED;
        }
        else
        {
            P_ERROR("[ERROR]: Cannot remove file %s, file status is %d\n", m_name, status);
        }
    }
    else
    {
        status = FFS_F_UNAVAILABLE;
        P_ERROR("[ERROR]: removing file on a not available file system\n");
    }
}

// flush chached changes to the flash memory
void ICACHE_FLASH_ATTR Ffile::flush_cache()
{
    if (m_fs && (m_fs->is_available()))
    {
        if (status == FFS_F_MODIFIED_UNSAVED)
        {
            s32_t res = SPIFFS_fflush(m_fs->get_handler(), m_fd);
            if (res < SPIFFS_OK)
                P_ERROR("[ERROR]: Error %d while flushing cache for file %s\n", SPIFFS_errno(m_fs->get_handler()), m_name);
        }
        else
        {
            P_ERROR("[ERROR]: Cannot flush cache for file %s, file status is %d\n", m_name, status);
        }
    }
    else
    {
        status = FFS_F_UNAVAILABLE;
        P_ERROR("[ERROR]: flushing cache on a not available file system\n");
    }
}

bool ICACHE_FLASH_ATTR Ffile::exists(Flashfs *t_fs, char *t_name)
{
    spiffs_DIR directory;
    struct spiffs_dirent tmp_file;
    struct spiffs_dirent *file_ptr;

    if (t_fs->is_available())
    {
        SPIFFS_opendir(t_fs->get_handler(), "/", &directory);
        while ((file_ptr = SPIFFS_readdir(&directory, &tmp_file)))
        {
            if (0 == os_strncmp(t_name, (char *)file_ptr->name, os_strlen(t_name)))
            {
                return true;
            }
        }
        SPIFFS_closedir(&directory);
    }
    else
    {
        P_ERROR("[ERROR]: checking if file exists on not available file system\n");
    }
    return false;
}

int ICACHE_FLASH_ATTR Ffile::size(Flashfs *t_fs, char *t_name)
{
    spiffs_DIR directory;
    struct spiffs_dirent tmp_file;
    struct spiffs_dirent *file_ptr;

    if (t_fs->is_available())
    {
        SPIFFS_opendir(t_fs->get_handler(), "/", &directory);
        while ((file_ptr = SPIFFS_readdir(&directory, &tmp_file)))
        {
            if (0 == os_strncmp(t_name, (char *)file_ptr->name, os_strlen(t_name)))
            {
                return file_ptr->size;
            }
        }
        SPIFFS_closedir(&directory);
    }
    else
    {
        P_ERROR("[ERROR]: checking file size on not available file system\n");
    }
    return -1;
}