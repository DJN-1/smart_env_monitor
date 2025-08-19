CC = gcc
CFLAGS = -Wall -g -I../../include -D_POSIX_C_SOURCE=200809L
LIBS = -lgpiod

SOURCES = smart_env_ui.c \
          ../../drivers/dht11_sensor.c \
          ../../drivers/ds1307_rtc.c \
          ../../drivers/gpio_driver.c \
          ../../drivers/gpio_control.c

TARGET = smart_env_ui

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)

setup-driver:
	sudo insmod ../../drivers/oled_driver.ko || echo "모듈 이미 로드됨"
	echo "ssd1306 0x3c" | sudo tee /sys/bus/i2c/devices/i2c-1/new_device
	sudo chmod 666 /dev/oled_display

test: $(TARGET)
	sudo ./$(TARGET)

.PHONY: all clean setup-driver test
