#ifndef __USER_HUMITURE_H__
#define __USER_HUMITURE_H__

#include "driver/i2c_master.h"
#include "driver/key.h"

#define MVH3004_Addr    0x88

#define HUMITURE_KEY_NUM    1

#define HUMITURE_KEY_IO_MUX     PERIPHS_IO_MUX_MTCK_U
#define HUMITURE_KEY_IO_NUM     13
#define HUMITURE_KEY_IO_FUNC    FUNC_GPIO13

bool user_mvh3004_read_th(uint8 *data);
void user_mvh3004_init(void);

void user_humiture_init(void);

#endif
