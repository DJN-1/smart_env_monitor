#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "smart_env_monitor.h"

// OLED 디스플레이 설정
#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_PAGE_SIZE    8

// OLED 명령어
#define OLED_CMD_DISPLAY_OFF    0xAE
#define OLED_CMD_DISPLAY_ON     0xAF
#define OLED_CMD_SET_CONTRAST   0x81
#define OLED_CMD_CLEAR_DISPLAY  0x01

// 디스플레이 모드
typedef enum {
    DISPLAY_TEMPERATURE,
    DISPLAY_HUMIDITY,
    DISPLAY_TIME,
    DISPLAY_ALL
} display_mode_t;

// OLED 디스플레이 함수
int oled_init(void);
void oled_cleanup(void);
int oled_clear(void);
int oled_write_command(unsigned char cmd);
int oled_write_data(unsigned char data);
int oled_set_cursor(int x, int y);
int oled_print_string(const char *str);
int oled_display_sensor_data(const sensor_data_t *data, display_mode_t mode);

// 폰트 및 그래픽
int oled_draw_pixel(int x, int y);
int oled_draw_line(int x1, int y1, int x2, int y2);
void oled_update_display(void);

#endif // OLED_DISPLAY_H
