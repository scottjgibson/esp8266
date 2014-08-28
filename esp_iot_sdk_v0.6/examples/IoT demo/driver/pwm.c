/******************************************************************************
 * Copyright 本软件为网上搜集  (Wuxi)
 *
 * FileName: pwm.c
 *
 * Description: pwm driver
 *
 * 淘宝店铺http://anxinke.taobao.com/?spm=2013.1.1000126.d21.FqkI2r:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"

#include "user_interface.h"
#include "driver/pwm.h"

LOCAL struct pwm_single_param pwm_single[PWM_CHANNEL];
LOCAL struct pwm_period_param pwm_period;

LOCAL uint8 pwm_out_io_num[PWM_CHANNEL] = {PWM_0_OUT_IO_NUM, PWM_1_OUT_IO_NUM, PWM_2_OUT_IO_NUM};

/******************************************************************************
 * FunctionName : pwm_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : channel index
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_set_duty(uint8 duty, uint8 channel)
{
    if (duty < 1) {
        pwm_single[channel].duty = 0;
    } else if (duty >= PWM_DEPTH) {
        pwm_single[channel].duty = PWM_DEPTH;
    } else {
        pwm_single[channel].duty = duty;
    }

    pwm_single[channel].h_time = pwm_period.period * pwm_single[channel].duty / PWM_DEPTH;
}

/******************************************************************************
 * FunctionName : pwm_set_freq
 * Description  : set pwm frequency
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_set_freq(uint16 freq)
{
    uint8 i;

    if (freq > 500) {
        pwm_period.freq = 500;
    } else if (freq < 1) {
        pwm_period.freq = 1;
    } else {
        pwm_period.freq = freq;
    }

    pwm_period.period = PWM_1S / pwm_period.freq;

    for (i = 0; i < PWM_CHANNEL; i++) {
        pwm_single[i].h_time  = pwm_period.period * pwm_single[i].duty / PWM_DEPTH;
    }

    os_timer_disarm(&pwm_period.pwm_timer);
    os_timer_arm_us(&pwm_period.pwm_timer, pwm_period.period, 1);
}

/******************************************************************************
 * FunctionName : pwm_set_freq_duty
 * Description  : set pwm frequency and each channel's duty
 * Parameters   : uint16 freq : 100hz typically
 *                uint8 *duty : each channel's duty
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
pwm_set_freq_duty(uint16 freq, uint8 *duty)
{
    uint8 i;

    if (freq > 500) {
        pwm_period.freq = 500;
    } else if (freq < 1) {
        pwm_period.freq = 1;
    } else {
        pwm_period.freq = freq;
    }

    pwm_period.period = PWM_1S / pwm_period.freq;

    for (i = 0; i < PWM_CHANNEL; i++) {
        if (duty[i] < 1) {
            pwm_single[i].duty = 0;
        } else if (duty[i] >= PWM_DEPTH) {
            pwm_single[i].duty = PWM_DEPTH;
        } else {
            pwm_single[i].duty = duty[i];
        }

        pwm_single[i].h_time  = pwm_period.period * pwm_single[i].duty / PWM_DEPTH;
    }
}

/******************************************************************************
 * FunctionName : pwm_get_duty
 * Description  : get duty of each channel
 * Parameters   : uint8 channel : channel index
 * Returns      : NONE
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
pwm_get_duty(uint8 channel)
{
    return pwm_single[channel].duty;
}

/******************************************************************************
 * FunctionName : pwm_get_freq
 * Description  : get pwm frequency
 * Parameters   : NONE
 * Returns      : uint16 : pwm frequency
*******************************************************************************/
uint16 ICACHE_FLASH_ATTR
pwm_get_freq(void)
{
    return pwm_period.freq;
}

/******************************************************************************
 * FunctionName : pwm_output_low
 * Description  : each channel's high level timer function,
 *                after reach the timer, output low level.
 * Parameters   : uint8 channel : channel index
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
pwm_output_low(uint8 channel)
{
    PWM_OUTPUT_LOW(pwm_out_io_num[channel]);
}

/******************************************************************************
 * FunctionName : pwm_period_timer
 * Description  : pwm period timer function, output high level,
 *                start each channel's high level timer
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
pwm_period_timer(void)
{
    uint8 i;

    ETS_INTR_LOCK();

    for (i = 0; i < PWM_CHANNEL; i++) {
        if (pwm_single[i].h_time != 0) {
            PWM_OUTPUT_HIGH(pwm_out_io_num[i]);

            if (pwm_single[i].h_time != pwm_period.period) {
                os_timer_disarm(&pwm_single[i].pwm_timer);
                os_timer_arm_us(&pwm_single[i].pwm_timer, pwm_single[i].h_time, 0);
            }
        } else {
            PWM_OUTPUT_LOW(pwm_out_io_num[i]);
        }
    }

    ETS_INTR_UNLOCK();
}

/******************************************************************************
 * FunctionName : pwm_init
 * Description  : pwm gpio, params and timer initialization
 * Parameters   : uint16 freq : pwm freq param
 *                uint8 *duty : each channel's duty
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_init(uint16 freq, uint8 *duty)
{
    uint8 i;

    /* pwm needs precise timer, so define USE_US_TIMER in Makefile!!! */
    system_timer_reinit();

    PIN_FUNC_SELECT(PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC);
    PIN_FUNC_SELECT(PWM_1_OUT_IO_MUX, PWM_1_OUT_IO_FUNC);
    PIN_FUNC_SELECT(PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC);

    pwm_set_freq_duty(freq, duty);

    /* init each channel's high level timer of pwm. */
    for (i = 0; i < PWM_CHANNEL; i++) {
        os_timer_disarm(&pwm_single[i].pwm_timer);
        os_timer_setfn(&pwm_single[i].pwm_timer, (os_timer_func_t *)pwm_output_low, i);
    }

    /* init period timer of pwm. */
    os_timer_disarm(&pwm_period.pwm_timer);
    os_timer_setfn(&pwm_period.pwm_timer, (os_timer_func_t *)pwm_period_timer, NULL);
    os_timer_arm_us(&pwm_period.pwm_timer, pwm_period.period, 1);
}
