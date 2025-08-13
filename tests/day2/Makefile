CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -I../../include -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
LIBS = -lgpiod

TARGETS = ds1307_test dht11_sensor_test sensor_collector_test

all: $(TARGETS)

ds1307_test: ds1307_test.c ../../drivers/ds1307_rtc.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

dht11_sensor_test: dht11_sensor_test.c ../../drivers/dht11_sensor.c ../../drivers/gpio_driver.c ../../drivers/gpio_control.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

sensor_collector_test: sensor_collector_test.c \
    ../../drivers/sensor_collector.c \
    ../../drivers/dht11_sensor.c \
    ../../drivers/ds1307_rtc.c \
    ../../drivers/gpio_driver.c \
    ../../drivers/gpio_control.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGETS)

test: ds1307_test dht11_sensor_test
	@echo "🧪 DS1307 라이브러리 기반 테스트 실행..."
	./ds1307_test
	@echo "🌡️ DHT11 센서 테스트 실행..."
	./dht11_sensor_test

.PHONY: all clean test
