CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
LIBS = -lgpiod

TARGETS = gpio_basic_test oled_test ds1307_test ds1307_set_time dht11_test rotary_test

all: $(TARGETS)

gpio_basic_test: gpio_basic_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

oled_test: oled_test.c
	$(CC) $(CFLAGS) -o $@ $<

ds1307_test: ds1307_test.c
	$(CC) $(CFLAGS) -o $@ $<

ds1307_set_time: ds1307_set_time.c
	$(CC) $(CFLAGS) -o $@ $<

dht11_test: dht11_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

rotary_test: rotary_test.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGETS)

test: gpio_basic_test
	@echo "🚀 GPIO 기본 테스트 실행..."
	sudo ./gpio_basic_test

.PHONY: all clean test
