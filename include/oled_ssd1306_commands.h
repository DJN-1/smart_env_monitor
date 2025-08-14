#ifndef OLED_SSD1306_COMMANDS_H
#define OLED_SSD1306_COMMANDS_H

#include <linux/i2c.h>
#include <linux/types.h>  // u8 타입을 위해 필요

// OLED 초기화
int ssd1306_init_display(struct i2c_client *client);

// OLED 화면 지우기
int ssd1306_clear_display(struct i2c_client *client);

// OLED 디스플레이 켜기/끄기
int ssd1306_display_on(struct i2c_client *client);
int ssd1306_display_off(struct i2c_client *client);

// OLED 대비 설정
int ssd1306_set_contrast(struct i2c_client *client, u8 contrast);

// 텍스트 렌더링 (8x8 폰트 기준)
int ssd1306_render_text(struct i2c_client *client, const char *text, int page);

#endif  // OLED_SSD1306_COMMANDS_H
