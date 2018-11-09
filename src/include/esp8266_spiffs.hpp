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

// debug macros
#define P_FATAL(...) os_printf(__VA_ARGS__)
#define P_ERROR(...) os_printf(__VA_ARGS__)
#define P_WARN(...) os_printf(__VA_ARGS__)
#define P_INFO(...) os_printf(__VA_ARGS__)
#define P_DEBUG(...) os_printf(__VA_ARGS__)
#define P_TRACE(...) os_printf(__VA_ARGS__)
#define P_ALL(...) os_printf(__VA_ARGS__)

// SPIFFS cpp wrapper customised for ESP8266 and SDK

extern "C"
{
#include "spiffs.h"
}

#define SPIFFS_CACHE 0

class flafs
{
protected:
  // file system
  static const int LOG_PAGE_SIZE = 256;
  static const int FLASH_SECT_SIZE = (1024 * 4);

  u8_t m_work[LOG_PAGE_SIZE * 2]; //  512 bytes
  u8_t m_fd_space[32 * 8];        //  256 bytes => max 8 files
                                  //  768 bytes total without cache
  // NO CACHE. To enable cache <#define SPIFFS_CACHE 1> and uncomment following line for buffer definition
  // u8_t m_cache[(LOG_PAGE_SIZE + 32) * 4]; // 1152 bytes => cache
  // 1920 bytes total with cache

  spiffs *m_fs;                         // file system handler
  spiffs_config *m_config;              // file system configuration
  struct spiffs_dirent *m_current_file; // used for listing files
  spiffs_check_callback check_cb_f();

  // flash memory functions
  s32_t spiffs_read(u32_t t_addr, u32_t t_size, u8_t *t_dst);
  s32_t spiffs_write(u32_t t_addr, u32_t t_size, u8_t *t_src);
  s32_t spiffs_erase(u32_t t_addr, u32_t t_size);

public:
  flafs(){};
  ~flafs(){};
  void init(void); // will mount the file system
  void mount(void);
  void umount(void);
  void format(void);
  u32_t get_total_size();
  u32_t get_available_size();
  u32_t get_used_size();
  struct spiffs_dirent *list(int); // (0) => return first file information
                                   // (1) => return next file information
};

class ffile
{
private:
  spiffs *m_fs;
  char m_name[32];
  char status;

public:
  ffile(spiffs *, char *){};
  ~ffile(){};
  char get_status();
  void read(char *);
  void n_read(char *, int);
  void write(char *);
  void append(char *);
  void rename();
  void remove();
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