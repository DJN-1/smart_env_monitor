#ifndef OLED_SSD1306_COMMANDS_H
#define OLED_SSD1306_COMMANDS_H

#include <linux/i2c.h>
#include <linux/types.h>

// OLED 기본 함수들
int ssd1306_init_display(struct i2c_client *client);
int ssd1306_clear_display(struct i2c_client *client);
int ssd1306_display_on(struct i2c_client *client);
int ssd1306_display_off(struct i2c_client *client);
int ssd1306_set_contrast(struct i2c_client *client, u8 contrast);

// 텍스트 렌더링 함수들
int ssd1306_render_text_autowrap(struct i2c_client *client, const char *text);
int ssd1306_render_text(struct i2c_client *client, const char *text, int page);
int ssd1306_set_cursor(struct i2c_client *client, int col, int page);
int ssd1306_print_at(struct i2c_client *client, const char *text, int col, int page);
int ssd1306_print_multiline(struct i2c_client *client, const char *lines[], int num_lines);

// 테스트 함수
int ssd1306_test_pattern(struct i2c_client *client);

#endif // OLED_SSD1306_COMMANDS_H
