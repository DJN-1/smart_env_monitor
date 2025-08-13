#include <stdio.h>
#include <unistd.h>  // sleep()
#include "dht11_sensor.h"
#include "gpio_driver.h"
#include "smart_env_monitor.h"  // GPIO_DHT11_DATA 정의용

int main(void) {
    if (gpio_init() != 0) {
        fprintf(stderr, "GPIO 초기화 실패\n");
        return 1;
    }

    if (dht11_init(GPIO_DHT11_DATA) != 0) {
        fprintf(stderr, "DHT11 초기화 실패\n");
        gpio_cleanup();
        return 1;
    }

    dht11_data_t data;

    for (int i = 0; i < 10; i++) {  // 10회 반복 측정
        if (!dht11_is_ready_to_read()) {
            printf("⏳ 센서가 아직 읽을 준비가 안 됨 (2초 간격 필요)\n");
            sleep(1);  // 1초 대기 후 다시 확인
            continue;
        }

        if (dht11_read_data(&data) != 0) {
            fprintf(stderr, "❌ DHT11 데이터 읽기 실패\n");
        } else {
            dht11_print_data(&data);
        }

        sleep(2);  // 다음 측정까지 2초 대기
    }

    gpio_cleanup();
    return 0;
}
