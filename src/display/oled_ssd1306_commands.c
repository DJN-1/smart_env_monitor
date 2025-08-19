#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/errno.h>
#include "../include/font_data.h"

// SSD1306 명령어 정의
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_SEGREMAP_NORMAL     0xA0 // 수정: 표준 모드
#define SSD1306_SEGREMAP_REVERSE    0xA1
#define SSD1306_COMSCAN_NORMAL      0xC8 // 수정: 표준 모드
#define SSD1306_COMSCAN_REVERSE     0xC0
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SET_COLUMN_ADDR     0x21
#define SSD1306_SET_PAGE_ADDR       0x22

/**
 * ssd1306_init_display - OLED 디스플레이를 초기화합니다.
 * @client: I2C 클라이언트 포인터
 *
 * 하드웨어 플립/반전 기능 없이 가장 표준적인 모드로 OLED를 설정합니다.
 */
int ssd1306_init_display(struct i2c_client *client)
{
    // 명령어와 데이터를 한 번에 보내기 위해 재구성
    static const u8 init_sequence[] = {
        0x00, SSD1306_DISPLAYOFF,
        0x00, SSD1306_SETDISPLAYCLOCKDIV, 0x80,
        0x00, SSD1306_SETMULTIPLEX, 0x3F,
        0x00, SSD1306_SETDISPLAYOFFSET, 0x00,
        0x00, SSD1306_SETSTARTLINE,
        0x00, SSD1306_CHARGEPUMP, 0x14,
        0x00, SSD1306_MEMORYMODE, 0x00, // 0x00 = Horizontal Addressing Mode
        0x00, SSD1306_SEGREMAP_REVERSE,  // 좌우 반전 없음
        0x00, SSD1306_COMSCAN_NORMAL,   // 상하 반전 없음
        0x00, SSD1306_SETCOMPINS, 0x12,
        0x00, SSD1306_SETCONTRAST, 0xCF,
        0x00, SSD1306_SETPRECHARGE, 0xF1,
        0x00, SSD1306_SETVCOMDETECT, 0x40,
        0x00, SSD1306_DISPLAYALLON_RESUME,
        0x00, SSD1306_NORMALDISPLAY,
        0x00, SSD1306_DISPLAYON
    };
    int ret;

    pr_info("smart_env: SSD1306 초기화 시작...\n");
    ret = i2c_master_send(client, init_sequence, sizeof(init_sequence));
    if (ret < 0) {
        pr_err("smart_env: 초기화 실패 (ret: %d)\n", ret);
        return ret;
    }

    pr_info("smart_env: SSD1306 초기화 완료!\n");
    return 0;
}

/**
 * ssd1306_clear_display - 화면 전체를 0으로 채워 지웁니다.
 */
int ssd1306_clear_display(struct i2c_client *client)
{
    u8 page_buffer[129];
    int ret, page;

    // 데이터 버퍼의 첫 바이트는 데이터 전송을 의미하는 0x40
    page_buffer[0] = 0x40;
    memset(&page_buffer[1], 0x00, 128);

    for (page = 0; page < 8; page++) {
        u8 set_page_cmd[] = {0x00, 0xB0 | page};
        i2c_master_send(client, set_page_cmd, sizeof(set_page_cmd));
        ret = i2c_master_send(client, page_buffer, sizeof(page_buffer));
        if (ret < 0) {
            pr_err("smart_env: 페이지 %d 클리어 실패\n", page);
            return ret;
        }
    }

    pr_info("smart_env: OLED 화면 지우기 완료\n");
    return 0;
}

/**
 * ssd1306_display_on - 디스플레이를 켭니다.
 */
int ssd1306_display_on(struct i2c_client *client)
{
    u8 cmd[] = {0x00, SSD1306_DISPLAYON};
    return i2c_master_send(client, cmd, sizeof(cmd));
}

/**
 * ssd1306_display_off - 디스플레이를 끕니다.
 */
int ssd1306_display_off(struct i2c_client *client)
{
    u8 cmd[] = {0x00, SSD1306_DISPLAYOFF};
    return i2c_master_send(client, cmd, sizeof(cmd));
}

/**
 * ssd1306_set_contrast - 명암을 조절합니다.
 */
int ssd1306_set_contrast(struct i2c_client *client, u8 contrast)
{
    u8 cmd[] = {0x00, SSD1306_SETCONTRAST, contrast};
    return i2c_master_send(client, cmd, sizeof(cmd));
}

/**
 * ssd1306_render_text - 지정된 페이지(줄)에 텍스트를 수평으로 출력합니다.
 * @client: I2C 클라이언트 포인터
 * @text: 출력할 문자열
 * @page: 출력할 페이지 (0~7)
 *
 * 폰트 회전 없이 가로로 텍스트를 그리는 표준 방식입니다.
 */

int ssd1306_render_text(struct i2c_client *client,
                        const char *text,
                        int page)
{
    int i, j, ret;
    int text_len = strlen(text);
    u8 page_buffer[129];
    u8 set_pos_cmds[5];
    
    // --- 추가: 좌우 반전된 폰트를 임시 저장할 버퍼 ---
    u8 flipped_char_buffer[8];

    if (page < 0 || page > 7) {
        pr_err("smart_env: 잘못된 페이지 번호: %d\n", page);
        return -EINVAL;
    }

    page_buffer[0] = 0x40;
    memset(&page_buffer[1], 0x00, 128);

    for (i = 0; i < text_len; i++) {
        char c = text[i];
        if (c < 32 || c > 127) c = '?';

        int x_pos = i * 6;
        if (x_pos + 6 > 128) break;

        const u8 *original_font_char = font6x8_basic[c - 32];
        
        // --- 추가: 폰트 데이터를 먼저 좌우 반전시킵니다 ---
        flip_font_6x8_horizontal(original_font_char, flipped_char_buffer);

        // 폰트의 8개 행(row) 데이터를 6개 열(column)에 맞게 재구성
        for (j = 0; j < 6; j++) {
            u8 col_data = 0;
            // --- 수정: 반전된 폰트(flipped_char_buffer)를 사용합니다 ---
            if ((flipped_char_buffer[0] >> j) & 1) col_data |= (1 << 0);
            if ((flipped_char_buffer[1] >> j) & 1) col_data |= (1 << 1);
            if ((flipped_char_buffer[2] >> j) & 1) col_data |= (1 << 2);
            if ((flipped_char_buffer[3] >> j) & 1) col_data |= (1 << 3);
            if ((flipped_char_buffer[4] >> j) & 1) col_data |= (1 << 4);
            if ((flipped_char_buffer[5] >> j) & 1) col_data |= (1 << 5);
            if ((flipped_char_buffer[6] >> j) & 1) col_data |= (1 << 6);
            if ((flipped_char_buffer[7] >> j) & 1) col_data |= (1 << 7);
            page_buffer[1 + x_pos + j] = col_data;
        }
    }

    // OLED 커서 이동 및 데이터 전송 로직 (이하 동일)
    set_pos_cmds[0] = 0x00;
    set_pos_cmds[1] = 0xB0 | page;
    set_pos_cmds[2] = 0x00;
    set_pos_cmds[3] = 0x10;

    ret = i2c_master_send(client, set_pos_cmds, 4);
    if (ret < 0) return ret;

    ret = i2c_master_send(client, page_buffer, sizeof(page_buffer));

    return ret;
}

/**
 * ssd1306_render_auto_wrapped - 화면에 텍스트를 자동 줄바꿈하여 출력합니다.
 */
int ssd1306_render_auto_wrapped(struct i2c_client *client,
                                const char *text)
{
    const int max_cols  = 21; // 128 / 6 = 21.33...
    const int max_lines = 8;
    char linebuf[max_cols + 1];
    int text_len = strlen(text);
    int idx = 0, line = 0, ret;

    if (!text || text_len == 0) return -EINVAL;

    ret = ssd1306_clear_display(client);
    if (ret < 0) return ret;

    while (idx < text_len && line < max_lines) {
        int copy_len = 0;

        while (copy_len < max_cols &&
               idx + copy_len < text_len &&
               text[idx + copy_len] != '\n') {
            copy_len++;
        }

        memcpy(linebuf, &text[idx], copy_len);
        linebuf[copy_len] = '\0';

        ret = ssd1306_render_text(client, linebuf, line);
        if (ret < 0) return ret;

        idx += copy_len;
        if (idx < text_len && text[idx] == '\n') idx++;
        line++;
    }

    return 0;
}
