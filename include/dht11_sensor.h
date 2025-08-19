#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include "smart_env_monitor.h"

// DHT11 센서 설정
#define DHT11_MAX_TIMINGS   85
#define DHT11_READ_TIMEOUT  10000  // 10ms
#define DHT11_MIN_INTERVAL  3000000 // 3초 (마이크로초)

// DHT11 데이터 구조체
typedef struct {
    float temperature;
    float humidity;
    int checksum_valid;
    struct timespec last_read;
} dht11_data_t;

// DHT11 센서 함수
int dht11_init(int gpio_pin);
void dht11_cleanup(void);
int dht11_read_data(dht11_data_t *data);
int dht11_is_ready_to_read(void);
void dht11_print_data(const dht11_data_t *data);

// 내부 함수 (1-Wire 통신)
int dht11_wait_for_state(int pin, int state, int timeout_us);
int dht11_read_bit(int pin);
int dht11_validate_checksum(unsigned char data[5]);

#endif // DHT11_SENSOR_H
