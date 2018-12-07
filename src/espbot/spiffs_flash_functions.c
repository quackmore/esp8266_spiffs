/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */

#include "c_types.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "spi_flash.h"

#include "spiffs_flash_functions.h"

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