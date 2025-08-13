#include "gpio_driver.h"
#include <gpiod.h>
#include <stdio.h>

struct gpiod_chip *gpio_chip = NULL;

int gpio_init(void) {
    if (!gpio_chip) {
        gpio_chip = gpiod_chip_open("/dev/gpiochip0");
        if (!gpio_chip) {
            perror("GPIO 칩 열기 실패");
            return -1;
        }
    }
    return 0;
}

void gpio_cleanup(void) {
    if (gpio_chip) {
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
    }
}
