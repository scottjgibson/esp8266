#ifndef __PWM_H__
#define __PWM_H__

struct pwm_single_param {
    os_timer_t pwm_timer;
    uint16 h_time;
    uint8 duty;
};

struct pwm_period_param {
    os_timer_t pwm_timer;
    uint16 period;
    uint16 freq;
};

#define PWM_DEPTH 255
#define PWM_CHANNEL 3

#define PWM_1S 1000000

#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_0_OUT_IO_NUM 12
#define PWM_0_OUT_IO_FUNC  FUNC_GPIO12

#define PWM_1_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 15
#define PWM_1_OUT_IO_FUNC  FUNC_GPIO15

#define PWM_2_OUT_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define PWM_2_OUT_IO_NUM 13
#define PWM_2_OUT_IO_FUNC  FUNC_GPIO13

#define PWM_OUTPUT_HIGH(pwm_out_io_num)  \
    gpio_output_set(1<<pwm_out_io_num, 0, 1<<pwm_out_io_num, 0)

#define PWM_OUTPUT_LOW(pwm_out_io_num)  \
    gpio_output_set(0, 1<<pwm_out_io_num, 1<<pwm_out_io_num, 0)

void pwm_init(uint16 freq, uint8 *duty);

void pwm_set_duty(uint8 duty, uint8 channel);
uint8 pwm_get_duty(uint8 channel);
void pwm_set_freq(uint16 freq);
uint16 pwm_get_freq(void);
#endif

