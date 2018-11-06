/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

uint32 priv_param_start_sec;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map)
    {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        priv_param_start_sec = 0x3C;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        priv_param_start_sec = 0x7C;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
        rf_cal_sec = 512 - 5;
        priv_param_start_sec = 0x7C;
        break;
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        priv_param_start_sec = 0xFC;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
        rf_cal_sec = 1024 - 5;
        priv_param_start_sec = 0x7C;
        break;
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        priv_param_start_sec = 0xFC;
        break;

    case FLASH_SIZE_64M_MAP_1024_1024:
        rf_cal_sec = 2048 - 5;
        priv_param_start_sec = 0xFC;
        break;
    case FLASH_SIZE_128M_MAP_1024_1024:
        rf_cal_sec = 4096 - 5;
        priv_param_start_sec = 0xFC;
        break;
    default:
        rf_cal_sec = 0;
        priv_param_start_sec = 0;
        break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

#ifndef SPI_FLASH_SIZE_MAP
#define SPI_FLASH_SIZE_MAP 4
#endif

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR 0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR 0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0xfd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0x7c000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR 0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0x7c000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR 0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0x7c000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR 0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0xfc000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR 0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR 0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR 0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR 0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0xfc000
#else
#error "The flash map is not supported"
#endif

#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM SYSTEM_PARTITION_CUSTOMER_BEGIN

static const partition_item_t at_partition_table[] = {
    {SYSTEM_PARTITION_BOOTLOADER, 0x0, 0x1000},
    {SYSTEM_PARTITION_OTA_1, 0x1000, SYSTEM_PARTITION_OTA_SIZE},
    {SYSTEM_PARTITION_OTA_2, SYSTEM_PARTITION_OTA_2_ADDR, SYSTEM_PARTITION_OTA_SIZE},
    {SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000},
    {SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000},
    {SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000},
    {SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM, SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR, 0x1000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if (!system_partition_table_regist(at_partition_table, sizeof(at_partition_table) / sizeof(at_partition_table[0]), SPI_FLASH_SIZE_MAP))
    {
        os_printf("system_partition_table_regist fail\r\n");
        while (1)
            ;
    }
}

/*
 * this is a C++ function defined in espbot.cpp
 */

void espbot_init(void);

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/

void ICACHE_FLASH_ATTR user_init(void)
{
    system_init_done_cb(espbot_init);
}
