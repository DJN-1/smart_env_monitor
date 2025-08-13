#ifndef SMART_ENV_MONITOR_H
#define SMART_ENV_MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>

// GPIO 라이브러리
#include <gpiod.h>

// 통신 인터페이스
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>

// 프로젝트 설정
#define PROJECT_NAME "Smart Environment Monitor"
#define VERSION "1.0.0"

// GPIO 핀 정의
#define GPIO_ROTARY_CLK    17
#define GPIO_ROTARY_DT     18
#define GPIO_ROTARY_SW     27
#define GPIO_DHT11_DATA    4

// I2C 설정
#define I2C_DEVICE         "/dev/i2c-1"
#define OLED_I2C_ADDR      0x3C

// SPI 설정  
#define SPI_DEVICE         "/dev/spidev0.0"
#define SPI_SPEED          1000000

// 함수 선언
int gpio_init(void);
void gpio_cleanup(void);
int sensors_init(void);
int display_init(void);
int communication_init(void);

#endif // SMART_ENV_MONITOR_H
