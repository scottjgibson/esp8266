#ifndef __USER_DEVICE_H__
#define __USER_DEVICE_H__

#define ESP_PARAM_SAVE_SEC       1

#define packet_size   (2 * 1024)

#define token_size 41

struct esp_platform_saved_param {
    uint8 devkey[40];
    uint8 token[40];
    uint8 activeflag;
    uint8 pad[3];
};

void user_esp_platform_restore(void);

#endif
