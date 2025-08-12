#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include "smart_env_monitor.h"

// GPIO 칩 전역 변수
extern struct gpiod_chip *gpio_chip;

// GPIO 기본 함수
int gpio_init(void);
void gpio_cleanup(void);
int gpio_set_output(int pin, int value);
int gpio_get_input(int pin);

// 로터리 스위치 구조체
typedef struct {
    struct gpiod_line *clk_line;
    struct gpiod_line *dt_line;
    struct gpiod_line *sw_line;
    int last_clk;
    int counter;
} rotary_switch_t;

// 로터리 스위치 함수
int rotary_switch_init(rotary_switch_t *rotary, int clk_pin, int dt_pin, int sw_pin);
void rotary_switch_cleanup(rotary_switch_t *rotary);
int rotary_switch_read(rotary_switch_t *rotary);

#endif // GPIO_CONTROL_H
