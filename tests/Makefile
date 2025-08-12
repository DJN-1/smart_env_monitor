CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L
LIBS = -lgpiod

TARGETS = gpio_basic_test i2c_test spi_test

all: $(TARGETS)

gpio_basic_test: gpio_basic_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

i2c_test: i2c_test.c
	$(CC) $(CFLAGS) -o $@ $

spi_test: spi_test.c
	$(CC) $(CFLAGS) -o $@ $

clean:
	rm -f $(TARGETS)

test: gpio_basic_test
	@echo "🚀 GPIO 기본 테스트 실행..."
	sudo ./gpio_basic_test

.PHONY: all clean test
