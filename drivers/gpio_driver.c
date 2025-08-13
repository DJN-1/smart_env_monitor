#include "gpio_driver.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define GPIO_CHIP "/dev/gpiochip0"

static struct gpiod_chip *chip = NULL;
static struct gpiod_line *lines[32] = {0};  // 최대 32핀 가정

int gpio_set_mode(int pin, int mode) {
    if (!chip) chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) return -1;

    if (lines[pin]) gpiod_line_release(lines[pin]);

    lines[pin] = gpiod_chip_get_line(chip, pin);
    if (!lines[pin]) return -1;

    int ret;
    if (mode == GPIO_MODE_OUTPUT) {
        ret = gpiod_line_request_output(lines[pin], "gpio_driver", GPIO_LOW);
    } else {
        ret = gpiod_line_request_input(lines[pin], "gpio_driver");
    }

    return (ret == 0) ? 0 : -1;
}

int gpio_write(int pin, int value) {
    if (!lines[pin]) return -1;
    return gpiod_line_set_value(lines[pin], value);
}

int gpio_read(int pin) {
    if (!lines[pin]) return -1;
    return gpiod_line_get_value(lines[pin]);
}

void gpio_delay_us(int microseconds) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);

    long target_ns = microseconds * 1000L;
    while(1){
	    clock_gettime(CLOCK_MONOTONIC, &now);
	    long elapsed_ns = (now.tv_sec - start.tv_sec) * 1000000000L +
		    (now.tv_nsec - start.tv_nsec);
	    if (elapsed_ns >= target_ns) break;
    }
}

int gpio_export(int pin) {
    // 선택적 구현: /sys/class/gpio 접근 방식
    return 0;
}

int gpio_unexport(int pin) {
    return 0;
}
