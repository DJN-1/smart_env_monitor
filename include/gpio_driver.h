#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <gpiod.h>
#include <time.h>

// GPIO 모드
#define GPIO_MODE_INPUT  0
#define GPIO_MODE_OUTPUT 1

// GPIO 값
#define GPIO_LOW  0
#define GPIO_HIGH 1

extern struct gpiod_chip *gpio_chip;

// 함수 선언
int gpio_set_mode(int pin, int mode);
int gpio_write(int pin, int value);
int gpio_read(int pin);
void gpio_delay_us(int microseconds);
int gpio_export(int pin);   // optional
int gpio_unexport(int pin); // optional

#endif // GPIO_DRIVER_H
