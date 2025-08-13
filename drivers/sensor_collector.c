#include "sensor_collector.h"
#include "dht11_sensor.h"
#include "ds1307_rtc.h"
#include "smart_env_monitor.h"  // GPIO_DHT11_DATA 정의
#include <time.h>

void sensor_collector_init(void) {
    gpio_init();
    dht11_init(GPIO_DHT11_DATA);
    ds1307_init();
}

int collect_sensor_data(sensor_data_t *result) {
    dht11_data_t dht;
    struct tm rtc_time;

    // DHT11 데이터 수집
    if (dht11_is_ready_to_read() && dht11_read_data(&dht) == 0 && dht.checksum_valid) {
        result->temperature = dht.temperature;
        result->humidity = dht.humidity;
        result->data_valid = 1;
    } else {
        result->temperature = 0.0f;
        result->humidity = 0.0f;
        result->data_valid = 0;
    }

    // RTC 시간 수집
    if (ds1307_read_time(&rtc_time) == 0) {
        result->timestamp = rtc_time;
    } else {
        // RTC 오류 시 현재 시스템 시간으로 대체
        time_t now = time(NULL);
        result->timestamp = *localtime(&now);
    }

    return result->data_valid;
}
