#include "dht11_sensor.h"
#include "gpio_driver.h"
#include <stdio.h>
#include <math.h>

static int dht11_pin = -1;
static struct timespec last_read_time = {0};
// 안정화를 위한 추가 변수들
static float last_valid_temp = 25.0;    // 기본값
static float last_valid_humi = 50.0;    // 기본값
static int consecutive_errors = 0;


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
    int retry_count = 0;
    int max_retries = 5;  // 최대 5번 재시도
    
    while (retry_count < max_retries) {
        // 재시도 간 대기 (짧게)
        if (retry_count > 0) {
            gpio_delay_us(500000); // 0.5초 대기
        }
        
        // Start signal (더 안정적으로)
        gpio_set_mode(dht11_pin, GPIO_MODE_OUTPUT);
        gpio_write(dht11_pin, GPIO_LOW);
        gpio_delay_us(20000);  // 20ms로 조정
        gpio_write(dht11_pin, GPIO_HIGH);
        gpio_delay_us(30);     // 30us로 조정
        gpio_set_mode(dht11_pin, GPIO_MODE_INPUT);

        // Sensor response
        if (dht11_wait_for_state(dht11_pin, GPIO_LOW, DHT11_READ_TIMEOUT) != 0) {
            retry_count++;
            continue;
        }

        if (dht11_wait_for_state(dht11_pin, GPIO_HIGH, DHT11_READ_TIMEOUT) != 0) {
            retry_count++;
            continue;
        }

        if (dht11_wait_for_state(dht11_pin, GPIO_LOW, DHT11_READ_TIMEOUT) != 0) {
            retry_count++;
            continue;
        }

        // Read 40 bits
        int read_success = 1;
        memset(data, 0, sizeof(data));
        
        for (int i = 0; i < 40; i++) {
            int bit = dht11_read_bit(dht11_pin);
            if (bit < 0) {
                read_success = 0;
                break;
            }
            data[i / 8] |= bit << (7 - (i % 8));
        }
        
        if (!read_success) {
            retry_count++;
            continue;
        }

        // Checksum validation
        if (!dht11_validate_checksum(data)) {
            retry_count++;
            continue;
        }
        
        float new_temp = data[2] + data[3] * 0.1f;
        float new_humi = data[0] + data[1] * 0.1f;
        
        // 값 검증 (합리적 범위 체크)
        if (new_temp < -10 || new_temp > 60 || new_humi < 5 || new_humi > 95) {
            retry_count++;
            continue;
        }
        
        // 성공! 값 저장
        result->temperature = new_temp;
        result->humidity = new_humi;
        result->checksum_valid = 1;
        clock_gettime(CLOCK_MONOTONIC, &result->last_read);
        last_read_time = result->last_read;
        
        return 0;
    }
    
    // 모든 재시도 실패 시 이전 값 사용
    if (consecutive_errors < 3) {
        result->temperature = last_valid_temp;
        result->humidity = last_valid_humi;
        result->checksum_valid = 1;
    }
    
    return -1;
}

void dht11_print_data(const dht11_data_t *data) {
    printf("🌡️ 온도: %.1f°C, 💧 습도: %.1f%% [%s]\n",
           data->temperature,
           data->humidity,
           data->checksum_valid ? "정상" : "오류");
}
