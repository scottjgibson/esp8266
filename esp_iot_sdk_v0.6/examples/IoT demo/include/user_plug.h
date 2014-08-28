#ifndef __USER_ESPSWITCH_H__
#define __USER_ESPSWITCH_H__

#include "driver/key.h"

/* user parameter save sector Number: 0~1, each sector is 4K */
#define PRIV_PARAM_SAVE_SEC     0

#define PLUG_KEY_NUM            1

#define PLUG_KEY_0_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define PLUG_KEY_0_IO_NUM     14
#define PLUG_KEY_0_IO_FUNC    FUNC_GPIO14

#define PLUG_RELAY_LED          GPIO_ID_PIN(12)

#define PLUG_STATUS_OUTPUT(pin, on)     GPIO_OUTPUT_SET(pin, on)

struct plug_saved_param {
    uint8_t status;
    uint8_t pad[3];
};

void user_plug_init(void);
uint8 user_plug_get_status(void);
void user_plug_set_status(bool status);


#endif

