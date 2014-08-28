/******************************************************************************
 * Copyright 本软件为网上搜集  (Wuxi)
 *
 * FileName: user_esp_platform.c
 *
 * Description: The client mode configration.
 *              Check your hardware connection with the host while use this mode.
 *
 * 淘宝店铺http://anxinke.taobao.com/?spm=2013.1.1000126.d21.FqkI2r:
 *     2014/5/09, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "espconn.h"
#include "user_esp_platform.h"

#if ESP_PLATFORM

#define ESP_DEBUG

#ifdef ESP_DEBUG
#define ESP_DBG os_printf
#else
#define ESP_DBG
#endif

#define ACTIVE_FRAME    "{\"path\": \"/v1/device/activate/\", \"method\": \"POST\", \"body\": {\"encrypt_method\": \"PLAIN\", \"token\": \"%s\"}, \"meta\": {\"Authorization\": \"token %s\",\"bssid\":\""MACSTR"\"}}\n"

#if PLUG_DEVICE
#include "user_plug.h"

#define RESPONSE_FRAME  "{\"status\": 200, \"datapoint\": {\"x\": %d}, \"nonce\": %d, \"is_query_device\": true}\n"
#define FIRST_FRAME     "{\"nonce\": 12306, \"path\": \"/v1/device/\", \"method\": \"GET\",\"meta\": {\"Authorization\": \"token %s\"}}\n"
#elif LIGHT_DEVICE
#include "user_light.h"

#define RESPONSE_FRAME  "{\"nonce\": %d, \"datapoint\": {\"x\": %d,\"y\": %d,\"z\": %d,\"k\": %d,\"l\": %d},\"is_query_device\": true}\n"
#define FIRST_FRAME     "{\"nonce\": 12306, \"path\": \"/v1/device/\", \"method\": \"GET\",\"meta\": {\"Authorization\": \"token %s\"}}\n"
#elif HUMITURE_DEVICE
#define UPLOAD_FRAME  "{\"nonce\": %d, \"path\": \"/v1/datastreams/tem_hum/datapoint/\", \"method\": \"POST\", \
\"body\": {\"datapoint\": {\"x\": %d,\"y\": %d}}, \"meta\": {\"Authorization\": \"token %s\"}}\n"

LOCAL uint32 count = 0;
#endif

#if PLUG_DEVICE || LIGHT_DEVICE
#define BEACON_FRAME    "{\"path\": \"/v1/ping/\", \"method\": \"POST\",\"meta\": {\"Authorization\": \"token %s\"}}\n"

LOCAL uint32 current_time;

LOCAL void user_device_sent_beacon(struct espconn *pespconn);
#endif

LOCAL os_timer_t client_timer;
LOCAL struct esp_platform_saved_param esp_param;

LOCAL void user_esp_platform_connect(struct espconn *pespconn);

/******************************************************************************
 * FunctionName : user_esp_platform_restore
 * Description  : restore the device at the initialization state
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_restore(void)
{
    uint8 devkey[40];

#if PLUG_DEVICE || LIGHT_DEVICE
    restore_user_param(PRIV_PARAM_SAVE_SEC);
#endif

    os_memcpy(devkey, esp_param.devkey, 40);
    os_memset(&esp_param, 0xFF, sizeof(struct esp_platform_saved_param));
    os_memcpy(esp_param.devkey, devkey, 40);

    restore_user_param(ESP_PARAM_SAVE_SEC);
    save_user_param(ESP_PARAM_SAVE_SEC, &esp_param, sizeof(struct esp_platform_saved_param));
}

/******************************************************************************
 * FunctionName : user_esp_platform_get_token
 * Description  : get the espressif's device token
 * Parameters   : token -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_get_token(uint8_t *token)
{
    if (token == NULL) {
        return;
    }

    os_memcpy(token, esp_param.token, sizeof(esp_param.token));
}

/******************************************************************************
 * FunctionName : user_esp_platform_set_token
 * Description  : save the token for the espressif's device
 * Parameters   : token -- the parame point which write the flash
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_set_token(uint8_t *token)
{
    if (token == NULL) {
        return;
    }

    esp_param.activeflag = 0;
    os_memcpy(esp_param.token, token, os_strlen(token));
    save_user_param(ESP_PARAM_SAVE_SEC, &esp_param, sizeof(struct esp_platform_saved_param));
}

/******************************************************************************
 * FunctionName : user_esp_platform_parse_nonce
 * Description  : parse the device nonce
 * Parameters   : pbuffer -- the recivce data point
 * Returns      : the nonce
*******************************************************************************/
int ICACHE_FLASH_ATTR
user_esp_platform_parse_nonce(char *pbuffer)
{
    char *pstr = NULL;
    char *pparse = NULL;
    char noncestr[11] = {0};
    int nonce = 0;
    pstr = (char *)os_strstr(pbuffer, "\"nonce\": ");

    if (pstr != NULL) {
        pstr += 9;
        pparse = (char *)os_strstr(pstr, ",");

        if (pparse != NULL) {
            os_memcpy(noncestr, pstr, pparse - pstr);
        } else {
            pparse = (char *)os_strstr(pstr, "}");

            if (pparse != NULL) {
                os_memcpy(noncestr, pstr, pparse - pstr);
            } else {
                pparse = (char *)os_strstr(pstr, "]");

                if (pparse != NULL) {
                    os_memcpy(noncestr, pstr, pparse - pstr);
                } else {
                    return 0;
                }
            }
        }

        nonce = atoi(noncestr);
    }

    return nonce;

}

/******************************************************************************
 * FunctionName : user_esp_platform_get_info
 * Description  : get and update the espressif's device status
 * Parameters   : pespconn -- the espconn used to connect with host
 *                pbuffer -- prossing the data point
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_get_info(struct espconn *pconn, uint8 *pbuffer)
{
    char *pbuf = NULL;
    int nonce = 0;

    pbuf = (char *)os_zalloc(packet_size);

    nonce = user_esp_platform_parse_nonce(pbuffer);

    if (pbuf != NULL) {
#if PLUG_DEVICE
        os_sprintf(pbuf, RESPONSE_FRAME, user_plug_get_status(), nonce);
#elif LIGHT_DEVICE
        os_sprintf(pbuf, RESPONSE_FRAME, nonce, user_light_get_freq(),
                   user_light_get_duty(LIGHT_RED), user_light_get_duty(LIGHT_GREEN),
                   user_light_get_duty(LIGHT_BLUE), 50);
#endif

        ESP_DBG("%s\n", pbuf);
        espconn_sent(pconn, pbuf, os_strlen(pbuf));
        os_free(pbuf);
        pbuf = NULL;
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_set_info
 * Description  : prossing the data and controling the espressif's device
 * Parameters   : pespconn -- the espconn used to connect with host
 *                pbuffer -- prossing the data point
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_set_info(struct espconn *pconn, uint8 *pbuffer)
{
#if PLUG_DEVICE
    char *pstr = NULL;
    pstr = (char *)os_strstr(pbuffer, "plug-status");

    if (pstr != NULL) {
        pstr = (char *)os_strstr(pbuffer, "body");

        if (pstr != NULL) {

            if (os_strncmp(pstr + 27, "1", 1) == 0) {
                user_plug_set_status(0x01);
            } else if (os_strncmp(pstr + 27, "0", 1) == 0) {
                user_plug_set_status(0x00);
            }
        }
    }

#elif LIGHT_DEVICE
    char *pstr = NULL;
    char *pdata = NULL;
    char *pbuf = NULL;
    char recvbuf[10];
    uint16 length = 0;
    uint16 data = 0;
    pstr = (char *)os_strstr(pbuffer, "\"path\": \"/v1/datastreams/light/datapoint/\"");

    if (pstr != NULL) {
        pstr = (char *)os_strstr(pbuffer, "{\"datapoint\": ");

        if (pstr != NULL) {
            pbuf = (char *)os_strstr(pbuffer, "}}");
            length = pbuf - pstr;
            length += 2;
            pdata = (char *)os_zalloc(length + 1);
            os_memcpy(pdata, pstr, length);

            pstr = os_strchr(pdata, 'x');

            if (pstr != NULL) {
                pstr += 4;
                pbuf = os_strchr(pstr, ',');

                if (pbuf != NULL) {
                    length = pbuf - pstr;
                    os_memset(recvbuf, 0, 10);
                    os_memcpy(recvbuf, pstr, length);
                    data = atoi(recvbuf);
                    user_light_set_freq(data);
                }
            }

            pstr = os_strchr(pdata, 'y');

            if (pstr != NULL) {
                pstr += 4;
                pbuf = os_strchr(pstr, ',');

                if (pbuf != NULL) {
                    length = pbuf - pstr;
                    os_memset(recvbuf, 0, 10);
                    os_memcpy(recvbuf, pstr, length);
                    data = atoi(recvbuf);
                    user_light_set_duty(data, 0);
                }
            }

            pstr = os_strchr(pdata, 'z');

            if (pstr != NULL) {
                pstr += 4;
                pbuf = os_strchr(pstr, ',');

                if (pbuf != NULL) {
                    length = pbuf - pstr;
                    os_memset(recvbuf, 0, 10);
                    os_memcpy(recvbuf, pstr, length);
                    data = atoi(recvbuf);
                    user_light_set_duty(data, 1);
                }
            }

            pstr = os_strchr(pdata, 'k');

            if (pstr != NULL) {
                pstr += 4;;
                pbuf = os_strchr(pstr, ',');

                if (pbuf != NULL) {
                    length = pbuf - pstr;
                    os_memset(recvbuf, 0, 10);
                    os_memcpy(recvbuf, pstr, length);
                    data = atoi(recvbuf);
                    user_light_set_duty(data, 2);
                }
            }

            os_free(pdata);
        }
    }

#endif

    user_esp_platform_get_info(pconn, pbuffer);
}

/******************************************************************************
 * FunctionName : user_esp_platform_reconnect
 * Description  : reconnect with host after get ip
 * Parameters   : pespconn -- the espconn used to reconnect with host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_reconnect(struct espconn *pespconn)
{
    struct ip_info ipconfig;

    ESP_DBG("user_esp_platform_reconnect\n");

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (ipconfig.ip.addr != 0) {
        pespconn->proto.tcp->client_port = espconn_port();
        user_esp_platform_connect(pespconn);
    } else {
        os_timer_arm(&client_timer, 10, 0);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_discon_cb
 * Description  : disconnect successfully with the host
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_discon_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_discon_cb\n");

    if (pespconn == NULL) {
        return;
    }

    pespconn->proto.tcp->client_port = espconn_port();

#if HUMITURE_DEVICE
    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
    os_timer_arm(&client_timer, 1000, 0);
#else
    user_esp_platform_reconnect(pespconn);
#endif
}

/******************************************************************************
 * FunctionName : user_esp_platform_discon
 * Description  : A new incoming connection has been disconnected.
 * Parameters   : espconn -- the espconn used to disconnect with host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_discon(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_discon\n");

    espconn_disconnect(pespconn);
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent_cb
 * Description  : Data has been sent successfully and acknowledged by the remote host.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_sent_cb\n");
}

/******************************************************************************
 * FunctionName : user_esp_platform_sent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent(struct espconn *pespconn)
{
    uint8 devkey[token_size] = {0};
    char *pbuf = (char *)os_zalloc(packet_size);

    os_memcpy(devkey, esp_param.devkey, 40);

    if (esp_param.activeflag == 0xFF) {
        esp_param.activeflag = 0;
    }

    if (pbuf != NULL) {
        if (esp_param.activeflag == 0) {
            uint8 token[token_size] = {0};
            uint8 bssid[6];

            os_memcpy(token, esp_param.token, 40);

            wifi_get_macaddr(STATION_IF, bssid);

            os_sprintf(pbuf, ACTIVE_FRAME, token, devkey, MAC2STR(bssid));
        }

#if HUMITURE_DEVICE
        else {
            uint16 tp, rh;
            uint8 data[4];

            if (user_mvh3004_read_th(data)) {
                rh = data[0] << 8 | data[1];
                tp = data[2] << 8 | data[3];
            }

            os_sprintf(pbuf, UPLOAD_FRAME, count, (tp >> 2) * 165 / (16384 - 1) - 40, (rh & 0x3fff) * 100 / (16384 - 1), devkey);
        }

#else
        else {
            os_sprintf(pbuf, FIRST_FRAME, devkey);
        }

#endif
        ESP_DBG("%s\n", pbuf);
        espconn_sent(pespconn, pbuf, os_strlen(pbuf));
        os_free(pbuf);
    }
}

#if PLUG_DEVICE || LIGHT_DEVICE
/******************************************************************************
 * FunctionName : user_esp_platform_sent_beacon
 * Description  : sent beacon frame for connection with the host is activate
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_sent_beacon(struct espconn *pespconn)
{
    if (pespconn == NULL) {
        return;
    }

    if (pespconn->state == ESPCONN_CONNECT) {
        if (esp_param.activeflag == 0) {
            ESP_DBG("plese check device is activated.\n");
            user_esp_platform_sent(pespconn);
        } else {
            uint8 devkey[token_size] = {0};
            os_memcpy(devkey, esp_param.devkey, 40);
            uint32 beacon_time = system_get_time();
            ESP_DBG("user_esp_platform_sent_beacon %d\n", beacon_time);

            if (beacon_time - current_time > BEACON_TIMEOUT) {
                ESP_DBG("user_esp_platform_sent_beacon sent fail\n");
                user_esp_platform_discon(pespconn);
            } else {
                char *pbuf = (char *)os_zalloc(packet_size);

                if (pbuf != NULL) {
                    os_sprintf(pbuf, BEACON_FRAME, devkey);
                    espconn_sent(pespconn, pbuf, os_strlen(pbuf));
                    os_timer_arm(&client_timer, BEACON_TIME, 0);
                    os_free(pbuf);
                }
            }
        }
    }
}

#endif

/******************************************************************************
 * FunctionName : user_esp_platform_recv_cb
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
    char *pstr = NULL;
    LOCAL char pbuffer[1024 * 2] = {0};
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_recv_cb %s\n", pusrdata);

    os_timer_disarm(&client_timer);

    if (length == 1024) {
        os_memcpy(pbuffer, pusrdata, length);
    } else {
        struct espconn *pespconn = (struct espconn *)arg;

        os_memcpy(pbuffer + os_strlen(pbuffer), pusrdata, length);

        if ((pstr = (char *)os_strstr(pbuffer, "\"activate_status\": ")) != NULL &&
                user_esp_platform_parse_nonce(pbuffer) != 12306) {
            if (os_strncmp(pstr + 19, "1", 1) == 0) {
                ESP_DBG("device activates successful.\n");

                esp_param.activeflag = 1;
                save_user_param(ESP_PARAM_SAVE_SEC, &esp_param, sizeof(struct esp_platform_saved_param));

                user_esp_platform_sent(pespconn);
            } else {
                ESP_DBG("device activates failed.\n");
            }
        }

#if (PLUG_DEVICE || LIGHT_DEVICE)
        else if ((pstr = (char *)os_strstr(pbuffer, "\"method\": ")) != NULL) {
            if (os_strncmp(pstr + 11, "GET", 3) == 0) {
                user_esp_platform_get_info(pespconn, pbuffer);
            } else if (os_strncmp(pstr + 11, "POST", 4) == 0) {
                user_esp_platform_set_info(pespconn, pbuffer);
            }

            os_timer_arm(&client_timer, BEACON_TIME, 0);
            current_time = system_get_time();
            ESP_DBG("user_esp_platform_recv_cb %d\n", current_time);
        } else if ((pstr = (char *)os_strstr(pbuffer, "ping success")) != NULL) {
            ESP_DBG("ping success\n");
            os_timer_arm(&client_timer, BEACON_TIME, 0);
            current_time = system_get_time();
            ESP_DBG("user_esp_platform_recv_cb %d\n", current_time);
        }

#elif HUMITURE_DEVICE
        else if ((pstr = (char *)os_strstr(pbuffer, "\"status\":")) != NULL) {
            if (os_strncmp(pstr + 10, "200", 3) != 0) {
                ESP_DBG("message upload failed.\n");
            } else {
                count++;
                ESP_DBG("message upload sucessful.\n");
            }

            os_timer_disarm(&client_timer);
            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_discon, pespconn);
            os_timer_arm(&client_timer, 10, 0);
        }

#endif
        else if ((pstr = (char *)os_strstr(pbuffer, "device")) != NULL) {
#if PLUG_DEVICE || LIGHT_DEVICE
            os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_sent_beacon, pespconn);
            os_timer_arm(&client_timer, BEACON_TIME, 0);
#elif HUMITURE_DEVICE

#endif
        }

        os_memset(pbuffer, 0, sizeof(pbuffer));
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_recon_cb
 * Description  : The connection had an error and is already deallocated.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_recon_cb(void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    ESP_DBG("user_esp_platform_recon_cb\n");

    os_timer_disarm(&client_timer);
    os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_reconnect, pespconn);
    os_timer_arm(&client_timer, 10000, 0);
}

/******************************************************************************
 * FunctionName : user_esp_platform_connect_cb
 * Description  : A new incoming connection has been connected.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;

    ESP_DBG("user_esp_platform_connect_cb\n");

    espconn_regist_disconcb(pespconn, user_esp_platform_discon_cb);
    espconn_regist_recvcb(pespconn, user_esp_platform_recv_cb);
    espconn_regist_sentcb(pespconn, user_esp_platform_sent_cb);
    user_esp_platform_sent(pespconn);
}

/******************************************************************************
 * FunctionName : user_esp_platform_connect
 * Description  : The function given as the connect with the host
 * Parameters   : espconn -- the espconn used to connect the connection
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_connect(struct espconn *pespconn)
{
    ESP_DBG("user_esp_platform_connect\n");

    espconn_connect(pespconn);
}

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : espconn struct parame init when get ip addr
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (ipconfig.ip.addr != 0) {
        struct espconn *light_espconn = NULL;

        light_espconn = (struct espconn *)os_zalloc(sizeof(struct espconn));

        if (light_espconn != NULL) {
            light_espconn->proto.tcp = NULL;
            light_espconn->type = ESPCONN_TCP;
            light_espconn->state = ESPCONN_NONE;
            light_espconn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

            if (light_espconn->proto.tcp != NULL) {
                const char sensor_ip[4] = {114, 215, 177, 97};

                os_memcpy(light_espconn->proto.tcp->ipaddr, sensor_ip, 4);
                light_espconn->proto.tcp->client_port = espconn_port();
                light_espconn->proto.tcp->server_port = 8000;

                espconn_regist_connectcb(light_espconn, user_esp_platform_connect_cb);
                espconn_regist_reconcb(light_espconn, user_esp_platform_recon_cb);
                user_esp_platform_connect(light_espconn);
            }
        }
    } else {
        os_timer_arm(&client_timer, 10, 0);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_init
 * Description  : device parame init based on espressif platform
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_esp_platform_init(void)
{
    load_user_param(ESP_PARAM_SAVE_SEC, &esp_param, sizeof(struct esp_platform_saved_param));

#if PLUG_DEVICE
    user_plug_init();
#elif LIGHT_DEVICE
    user_light_init();
#elif HUMITURE_DEVICE
    user_humiture_init();
#endif

    if (wifi_get_opmode() != SOFTAP_MODE) {
        os_timer_disarm(&client_timer);
        os_timer_setfn(&client_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
        os_timer_arm(&client_timer, 10, 0);
    }
}
#endif

