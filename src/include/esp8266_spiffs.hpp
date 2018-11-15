/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */
#ifndef __ESPBOT_SPIFFS_HPP__
#define __ESPBOT_SPIFFS_HPP__

// ESP8266 and SDK references
extern "C"
{
#include "user_config.h" // just for macros SPI_FLASH_SIZE_MAP and SYSTEM_PARTITION_RF_CAL_ADDR
                         // used for calculating spiffs size and location
}

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "There is no room for spiffs"
#elif (SPI_FLASH_SIZE_MAP == 2)
#error "There is no room for spiffs"
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_DATA 0x101000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_DATA 0x101000
#elif (SPI_FLASH_SIZE_MAP == 5)
#error "There is no room for spiffs"
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_DATA 0x201000
#else
#error "The flash map is not supported"
#endif

#define LOG_PAGE_SIZE (256)
#define FLASH_SECT_SIZE (1024 * 4)
#define FS_START SYSTEM_PARTITION_DATA
#define FS_END SYSTEM_PARTITION_RF_CAL_ADDR
#define FS_ALIGN_BYTES 4
#define SPIFFS_FLASH_RESULT_ERR -10200
#define SPIFFS_FLASH_RESULT_TIMEOUT -10201
#define SPIFFS_FLASH_BOUNDARY_ERROR -10202

// debug macros
#define debug (1)
#if debug
#define P_FATAL(...) os_printf(__VA_ARGS__)
#define P_ERROR(...) os_printf(__VA_ARGS__)
#define P_WARN(...) os_printf(__VA_ARGS__)
#define P_INFO(...) os_printf(__VA_ARGS__)
#define P_DEBUG(...) os_printf(__VA_ARGS__)
#define P_TRACE(...) os_printf(__VA_ARGS__)
#define P_ALL(...) os_printf(__VA_ARGS__)
#else
#define P_FATAL(...) \
  {                  \
  }
#define P_ERROR(...) \
  {                  \
  }
#define P_WARN(...) \
  {                 \
  }
#define P_INFO(...) \
  {                 \
  }
#define P_DEBUG(...) \
  {                  \
  }
#define P_TRACE(...) \
  {                  \
  }
#define P_ALL(...) \
  {                \
  }
#endif

// SPIFFS cpp wrapper customised for ESP8266 and SDK

extern "C"
{
#include "spiffs.h"
}

// flash memory functions
s32_t esp_spiffs_read(u32_t t_addr, u32_t t_size, u8_t *t_dst);
s32_t esp_spiffs_write(u32_t t_addr, u32_t t_size, u8_t *t_src);
s32_t esp_spiffs_erase(u32_t t_addr, u32_t t_size);

typedef enum
{
  FFS_AVAILABLE = 222,
  FFS_UNMOUNTED,
  FFS_UNAVAILABLE,
  FFS_CHECK_ERRORS
} flashfs_status;

class flashfs
{
protected:
  u8_t m_work[LOG_PAGE_SIZE * 2]; //  a ram memory buffer being double the size of the logical page size
  u8_t m_fd_space[32 * 4];        // 4 file descriptors => 4 file opened simultaneously
  // NO CACHE. To enable cache <#define SPIFFS_CACHE 1> in spiffs_config.h
#if SPIFFS_CACHE
  u8_t m_cache[(LOG_PAGE_SIZE + 32) * 4]; // 1152 bytes => cache
#else
  u8_t m_cache[1];
#endif
  // logical page buffer:     512 bytes
  // file descriptor buffer:  128 bytes =>  768 bytes total without cache
  // cache:                  1152 bytes => 1792 bytes total with cache
  // TIP: To get the exact amount of bytes needed on your specific target,
  // enable SPIFFS_BUFFER_HELP in spiffs_config.h, rebuild and call:
  //   SPIFFS_buffer_bytes_for_filedescs
  //   SPIFFS_buffer_bytes_for_cache

  spiffs m_fs;            // file system handler
  spiffs_config m_config; // file system configuration

  flashfs_status status;

public:
  flashfs(){};
  ~flashfs(){};
  void init(void);   // will mount the file system (eventually formatting it)
  void format(void); // will clean everything, a new init is required after this
  flashfs_status get_status();
  bool is_available();
  s32_t last_error();
  s32_t check();
  u32_t get_total_size();
  u32_t get_used_size();
  struct spiffs_dirent *list(int); // (0) => return first file information
                                   // (1) => return next file information
  spiffs *get_handle();
};

typedef enum
{
  FFS_F_UNAVAILABLE = 222,
  FFS_F_OPEN,
  FFS_F_MODIFIED_UNSAVED,
  FFS_F_REMOVED
} flashfs_file_status;

class ffile
{
private:
  spiffs *m_fs;
  spiffs_file m_fd;
  char m_name[32];
  flashfs_file_status status;

public:
  ffile(spiffs *);
  ffile(spiffs *, char *);
  ~ffile();
  char *get_name();
  void set_name(char *);
  flashfs_file_status get_status();
  int n_read(char *, int);
  int n_append(char *, int);
  void clear();
  void remove();
  void flush_cache();
  /*
      {
        ffile cfg(&fs, "config.cfg"); // constructor will open or create the file
        ...
        if (cfg.status == ERR)
          do something
        else if (cfg.status == JUST_CREATED)
          do something else
        else if (cfg.status == OPEN)
          do something else
         
        ...
                                      // destructor will close the file
      }
    */
};

#endif