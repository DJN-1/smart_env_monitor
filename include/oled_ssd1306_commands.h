#ifndef OLED_SSD1306_COMMANDS_H
#define OLED_SSD1306_COMMANDS_H

#include <linux/i2c.h>
#include <linux/types.h>

// 기본 OLED 제어 함수
int ssd1306_init_display(struct i2c_client *client);
int ssd1306_clear_display(struct i2c_client *client);
int ssd1306_display_on(struct i2c_client *client);
int ssd1306_display_off(struct i2c_client *client);
int ssd1306_set_contrast(struct i2c_client *client, u8 contrast);

// 텍스트 렌더링 함수
int ssd1306_render_text(struct i2c_client *client, const char *text, int page);

// --- 추가된 부분: 자동 줄바꿈 렌더링 함수 선언 ---
int ssd1306_render_auto_wrapped(struct i2c_client *client, const char *text);

// --- 제거된 부분: 미사용 멀티라인 함수 선언 ---
// int ssd1306_render_multiline(struct i2c_client *client, const char *lines[], int num_lines);

#endif  // OLED_SSD1306_COMMANDS_H
