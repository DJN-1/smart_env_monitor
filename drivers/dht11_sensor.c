#include "dht11_sensor.h"
#include "gpio_driver.h"
#include <stdio.h>

static int dht11_pin = -1;
static struct timespec last_read_time = {0};

int dht11_init(int gpio_pin) {
    dht11_pin = gpio_pin;
    clock_gettime(CLOCK_MONOTONIC, &last_read_time); // 초기화 시점 기록
    return gpio_set_mode(dht11_pin, GPIO_MODE_OUTPUT);
}

void dht11_cleanup(void) {
    // 필요 시 GPIO 해제
}

int dht11_is_ready_to_read(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    long elapsed_us = (now.tv_sec - last_read_time.tv_sec) * 1000000L +
                      (now.tv_nsec - last_read_time.tv_nsec) / 1000;

    return (elapsed_us >= DHT11_MIN_INTERVAL);
}

int dht11_wait_for_state(int pin, int state, int timeout_us) {
    while (timeout_us-- > 0) {
        if (gpio_read(pin) == state) return 0;
        gpio_delay_us(1);
    }
    return -1;
}

int dht11_read_bit(int pin) {
    if (dht11_wait_for_state(pin, GPIO_LOW, DHT11_READ_TIMEOUT) != 0) return -1;
    if (dht11_wait_for_state(pin, GPIO_HIGH, DHT11_READ_TIMEOUT) != 0) return -1;

    int count = 0;
    while (gpio_read(pin) == GPIO_HIGH) {
        gpio_delay_us(1);
        if (++count > 100) break;  // 100µs 이상이면 오류
    }

    return (count > 30) ? 1 : 0;  // 30µs 이상이면 1, 아니면 0
}

int dht11_validate_checksum(unsigned char data[5]) {
    unsigned char sum = data[0] + data[1] + data[2] + data[3];
    return (sum == data[4]);
}

int dht11_read_data(dht11_data_t *result) {
    unsigned char data[5] = {0};

    // Start signal
    gpio_set_mode(dht11_pin, GPIO_MODE_OUTPUT);
    gpio_write(dht11_pin, GPIO_LOW);
    gpio_delay_us(18000);  // ≥18ms
    gpio_write(dht11_pin, GPIO_HIGH);
    gpio_delay_us(40);
    gpio_set_mode(dht11_pin, GPIO_MODE_INPUT);

    // Sensor response
    if (dht11_wait_for_state(dht11_pin, GPIO_LOW, DHT11_READ_TIMEOUT) != 0) {
        fprintf(stderr, "❌ 센서 응답 LOW 실패 (1단계)\n");
        return -1;
    }

    if (dht11_wait_for_state(dht11_pin, GPIO_HIGH, DHT11_READ_TIMEOUT) != 0) {
        fprintf(stderr, "❌ 센서 응답 HIGH 실패 (2단계)\n");
        return -1;
    }

    if (dht11_wait_for_state(dht11_pin, GPIO_LOW, DHT11_READ_TIMEOUT) != 0) {
        fprintf(stderr, "❌ 센서 응답 LOW 실패 (3단계)\n");
        return -1;
    }

    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        int bit = dht11_read_bit(dht11_pin);
        if (bit < 0) {
            fprintf(stderr, "❌ 비트 %d 읽기 실패\n", i);
            return -1;
        }
        data[i / 8] |= bit << (7 - (i % 8));
    }

    // Checksum
    result->checksum_valid = dht11_validate_checksum(data);
    result->humidity = data[0] + data[1] * 0.1f;
    result->temperature = data[2] + data[3] * 0.1f;
    clock_gettime(CLOCK_MONOTONIC, &result->last_read);

    last_read_time = result->last_read;

    if (!result->checksum_valid) {
        fprintf(stderr, "⚠️ 체크섬 오류: %02X + %02X + %02X + %02X ≠ %02X\n",
                data[0], data[1], data[2], data[3], data[4]);
        return -1;
    }

    return 0;
}

void dht11_print_data(const dht11_data_t *data) {
    printf("🌡️ 온도: %.1f°C, 💧 습도: %.1f%% [%s]\n",
           data->temperature,
           data->humidity,
           data->checksum_valid ? "정상" : "오류");
}
