/******************************************************************************
 * Copyright 本软件为网上搜集  (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * 淘宝店铺http://anxinke.taobao.com/?spm=2013.1.1000126.d21.FqkI2r:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"

#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
#if ESP_PLATFORM
    user_esp_platform_init();
#endif

    user_devicefind_init();

    user_webserver_init(SERVER_PORT);
}

