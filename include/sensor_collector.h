#ifndef SENSOR_COLLECTOR_H
#define SENSOR_COLLECTOR_H

#include <time.h>

typedef struct {
    float temperature;
    float humidity;
    int rotary_position;
    int button_pressed;
    struct tm timestamp;
    int data_valid;
} sensor_data_t;

void sensor_collector_init(void);
int collect_sensor_data(sensor_data_t *result);

#endif // SENSOR_COLLECTOR_H
