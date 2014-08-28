#ifndef __USER_LIGHT_H__
#define __USER_LIGHT_H__

#include "driver/pwm.h"

/* user parameter save sector Number: 0~1, each sector is 4K */
#define PRIV_PARAM_SAVE_SEC     0

#define LIGHT_RED       0
#define LIGHT_GREEN     1
#define LIGHT_BLUE      2
#define LIGHE_LEVEL     3

struct light_saved_param {
    uint16 pwm_freq;
    uint8  pwm_duty[PWM_CHANNEL];
};

void user_light_init(void);
uint8 user_light_get_duty(uint8 channel);
void user_light_set_duty(uint8 duty, uint8 channel);
uint16 user_light_get_freq(void);
void user_light_set_freq(uint16 freq);

#endif

