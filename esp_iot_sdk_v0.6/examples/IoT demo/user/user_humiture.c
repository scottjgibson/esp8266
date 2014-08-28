/******************************************************************************
 * Copyright 本软件为网上搜集  (Wuxi)
 *
 * FileName: user_humiture.c
 *
 * Description: humiture demo's function realization
 *
 * 淘宝店铺http://anxinke.taobao.com/?spm=2013.1.1000126.d21.FqkI2r:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"

#include "user_humiture.h"

#if HUMITURE_DEVICE

LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[HUMITURE_KEY_NUM];

/******************************************************************************
 * FunctionName : user_mvh3004_burst_read
 * Description  : burst read mvh3004's internal data
 * Parameters   : uint8 addr - mvh3004's address
 *                uint8 *pData - data point to put read data
 *                uint16 len - read length
 * Returns      : bool - true or false
*******************************************************************************/
LOCAL bool ICACHE_FLASH_ATTR
user_mvh3004_burst_read(uint8 addr, uint8 *pData, uint16 len)
{
    uint8 ack;
    uint16 i;

    i2c_master_start();
    i2c_master_writeByte(addr);
    ack = i2c_master_getAck();

    if (ack) {
        os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }

    i2c_master_stop();
    i2c_master_wait(40000);

    i2c_master_start();
    i2c_master_writeByte(addr + 1);
    ack = i2c_master_getAck();

    if (ack) {
        os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }

    for (i = 0; i < len; i++) {
        pData[i] = i2c_master_readByte();

        i2c_master_setAck((i == (len - 1)) ? 1 : 0);
    }

    i2c_master_stop();

    return true;
}

/******************************************************************************
 * FunctionName : user_mvh3004_read_th
 * Description  : read mvh3004's humiture data
 * Parameters   : uint8 *data - where data to put
 * Returns      : bool - ture or false
*******************************************************************************/
bool ICACHE_FLASH_ATTR
user_mvh3004_read_th(uint8 *data)
{
    return user_mvh3004_burst_read(MVH3004_Addr, data, 4);
}

/******************************************************************************
 * FunctionName : user_mvh3004_init
 * Description  : init mvh3004, mainly i2c master gpio
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_mvh3004_init(void)
{
    i2c_master_gpio_init();
}

/******************************************************************************
 * FunctionName : user_humiture_long_press
 * Description  : humiture key's function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_humiture_long_press(void)
{
    system_restore();
    system_restart();
}

/******************************************************************************
 * FunctionName : user_humiture_init
 * Description  : init humiture function, include key and mvh3004
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_humiture_init(void)
{
    single_key[0] = key_init_single(HUMITURE_KEY_IO_NUM, HUMITURE_KEY_IO_MUX, HUMITURE_KEY_IO_FUNC,
                                    user_humiture_long_press, NULL);

    keys.key_num = HUMITURE_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);

    user_mvh3004_init();
}
#endif

