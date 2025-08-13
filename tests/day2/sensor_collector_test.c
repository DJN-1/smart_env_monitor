#include <stdio.h>
#include <unistd.h>
#include "sensor_collector.h"

int main(void) {
    sensor_collector_init();

    for (int i = 0; i < 10; i++) {
        sensor_data_t data;
        if (collect_sensor_data(&data)) {
            printf("📦 수집: %.1f°C, %.1f%% | %02d:%02d:%02d\n",
                   data.temperature,
                   data.humidity,
                   data.timestamp.tm_hour,
                   data.timestamp.tm_min,
                   data.timestamp.tm_sec);
        } else {
            fprintf(stderr, "⚠️ 센서 데이터 오류\n");
        }

        sleep(2);
    }

    return 0;
}
