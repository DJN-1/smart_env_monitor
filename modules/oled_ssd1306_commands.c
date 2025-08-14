#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/string.h>
#include "../include/font_data.h"

// SSD1306 명령어 정의
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_SEGREMAP            0xA1
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_COMSCANINC          0xC0
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_DISPLAYON           0xAF

// OLED 초기화
int ssd1306_init_display(struct i2c_client *client)
{
    u8 init_commands[] = {
        0x00, SSD1306_DISPLAYOFF,
        0x00, SSD1306_SETDISPLAYCLOCKDIV, 0x00, 0x80,
        0x00, SSD1306_SETMULTIPLEX, 0x00, 0x3F,
        0x00, SSD1306_SETDISPLAYOFFSET, 0x00, 0x00,
        0x00, SSD1306_SETSTARTLINE | 0x00,
        0x00, SSD1306_CHARGEPUMP, 0x00, 0x14,
        0x00, SSD1306_MEMORYMODE, 0x00, 0x00,        // 수평 주소 모드
        0x00, SSD1306_SEGREMAP | 0x00,               // 정상 세그먼트 방향
        0x00, SSD1306_COMSCANINC,                    // 정상 COM 스캔 방향
        0x00, SSD1306_SETCOMPINS, 0x00, 0x12,
        0x00, SSD1306_SETCONTRAST, 0x00, 0xCF,
        0x00, SSD1306_SETPRECHARGE, 0x00, 0xF1,
        0x00, SSD1306_SETVCOMDETECT, 0x00, 0x40,
        0x00, SSD1306_DISPLAYALLON_RESUME,
        0x00, SSD1306_NORMALDISPLAY,
        0x00, SSD1306_DISPLAYON
    };

    int ret, i;
    printk(KERN_INFO "smart_env: SSD1306 초기화 시작...\n");
    
    for (i = 0; i < sizeof(init_commands); i += 2) {
        ret = i2c_master_send(client, &init_commands[i], 2);
        if (ret < 0) {
            printk(KERN_ERR "smart_env: 초기화 실패 (index: %d, ret: %d)\n", i, ret);
            return ret;
        }
        msleep(1);
    }
    
    printk(KERN_INFO "smart_env: SSD1306 초기화 완료!\n");
    return 0;
}

// 화면 지우기
int ssd1306_clear_display(struct i2c_client *client)
{
    u8 clear_commands[] = {
        0x00, 0x21, 0x00, 0x00, 0x00, 0x7F,  // 열 주소 설정 (0-127)
        0x00, 0x22, 0x00, 0x00, 0x00, 0x07   // 페이지 주소 설정 (0-7)
    };

    u8 data_buffer[129];
    int ret, i, page;

    // 주소 설정 명령어 전송
    for (i = 0; i < sizeof(clear_commands); i += 2) {
        ret = i2c_master_send(client, &clear_commands[i], 2);
        if (ret < 0) {
            printk(KERN_ERR "smart_env: 클리어 명령어 전송 실패\n");
            return ret;
        }
    }

    // 데이터 버퍼 준비
    data_buffer[0] = 0x40; // 데이터 모드
    memset(&data_buffer[1], 0x00, 128);

    // 8개 페이지에 대해 데이터 전송
    for (page = 0; page < 8; page++) {
        ret = i2c_master_send(client, data_buffer, 129);
        if (ret < 0) {
            printk(KERN_ERR "smart_env: 페이지 %d 클리어 실패\n", page);
            return ret;
        }
    }

    printk(KERN_INFO "smart_env: OLED 화면 지우기 완료\n");
    return 0;
}

// 디스플레이 켜기
int ssd1306_display_on(struct i2c_client *client)
{
    u8 cmd[] = {0x00, SSD1306_DISPLAYON};
    int ret = i2c_master_send(client, cmd, 2);
    if (ret >= 0) {
        printk(KERN_INFO "smart_env: 디스플레이 켜짐\n");
    } else {
        printk(KERN_ERR "smart_env: 디스플레이 켜기 실패\n");
    }
    return ret;
}

// 디스플레이 끄기
int ssd1306_display_off(struct i2c_client *client)
{
    u8 cmd[] = {0x00, SSD1306_DISPLAYOFF};
    int ret = i2c_master_send(client, cmd, 2);
    if (ret >= 0) {
        printk(KERN_INFO "smart_env: 디스플레이 꺼짐\n");
    } else {
        printk(KERN_ERR "smart_env: 디스플레이 끄기 실패\n");
    }
    return ret;
}

// 대비 설정
int ssd1306_set_contrast(struct i2c_client *client, u8 contrast)
{
    u8 cmd[] = {0x00, SSD1306_SETCONTRAST, 0x00, contrast};
    int ret = i2c_master_send(client, cmd, 4);
    if (ret >= 0) {
        printk(KERN_INFO "smart_env: 대비 설정: %d\n", contrast);
    } else {
        printk(KERN_ERR "smart_env: 대비 설정 실패\n");
    }
    return ret;
}

// 텍스트 렌더링 함수 (90도 회전 보정 적용)
int ssd1306_render_text(struct i2c_client *client, const char *text, int page)
{
    u8 data[129];
    u8 rotated_char[8];  // 회전된 문자 버퍼
    int i, ret;
    int text_len = strlen(text);
    
    if (page < 0 || page > 7) {
        printk(KERN_ERR "smart_env: 잘못된 페이지 번호: %d (0-7 범위)\n", page);
        return -EINVAL;
    }

    if (text_len == 0) {
        printk(KERN_INFO "smart_env: 빈 텍스트, 렌더링 생략\n");
        return 0;
    }

    // 페이지 주소 설정
    u8 set_page[] = {0x00, 0xB0 | (page & 0x07)};
    ret = i2c_master_send(client, set_page, sizeof(set_page));
    if (ret < 0) {
        printk(KERN_ERR "smart_env: 페이지 설정 실패\n");
        return ret;
    }

    // 열 주소 설정 (시작 위치)
    u8 set_col[] = {0x00, 0x00, 0x00, 0x10}; // Lower/Higher Column Start Address
    ret = i2c_master_send(client, set_col, sizeof(set_col));
    if (ret < 0) {
        printk(KERN_ERR "smart_env: 열 주소 설정 실패\n");
        return ret;
    }

    // 텍스트 데이터 준비
    data[0] = 0x40; // 데이터 모드
    memset(&data[1], 0x00, 128); // 전체 라인 클리어

    // 문자별 렌더링 (최대 16문자, 8픽셀씩)
    for (i = 0; i < 16 && i < text_len; i++) {
        char c = text[i];
        
        // 지원하지 않는 문자는 '?'로 표시
        if (c < 32 || c > 127) {
            printk(KERN_WARNING "smart_env: 지원하지 않는 문자 '%c' -> '?'\n", c);
            c = '?';
        }
        
        // 🔧 핵심: 반시계방향 90도 회전으로 오른쪽 회전 보정!
        rotate_font_90_ccw(font8x8_basic[c - 32], rotated_char);
        
        // 회전된 폰트 데이터 복사 (8바이트씩)
        memcpy(&data[1 + i * 8], rotated_char, 8);
    }

    // 데이터 전송
    ret = i2c_master_send(client, data, 1 + (i * 8));
    if (ret >= 0) {
        printk(KERN_INFO "smart_env: 회전 보정 텍스트 렌더링 완료: '%s' (페이지 %d, %d문자)\n", 
               text, page, i);
    } else {
        printk(KERN_ERR "smart_env: 텍스트 렌더링 실패: %d\n", ret);
    }
    
    return ret;
}

// 커서 위치 설정 함수 (추가 기능)
int ssd1306_set_cursor(struct i2c_client *client, int col, int page)
{
    int ret;
    
    if (col < 0 || col > 127 || page < 0 || page > 7) {
	    printk(KERN_ERR "smart_env: 잘못된 커서 위치: col=%d, page=%d\n", col, page);
       return -EINVAL;
   }

   // 페이지 설정
   u8 set_page[] = {0x00, 0xB0 | (page & 0x07)};
   ret = i2c_master_send(client, set_page, sizeof(set_page));
   if (ret < 0) {
       printk(KERN_ERR "smart_env: 페이지 설정 실패\n");
       return ret;
   }

   // 열 위치 설정
   u8 set_col_low[] = {0x00, 0x00 | (col & 0x0F)};        // Lower 4 bits
   u8 set_col_high[] = {0x00, 0x10 | ((col >> 4) & 0x0F)}; // Higher 4 bits

   ret = i2c_master_send(client, set_col_low, sizeof(set_col_low));
   if (ret < 0) return ret;

   ret = i2c_master_send(client, set_col_high, sizeof(set_col_high));
   if (ret < 0) return ret;

   printk(KERN_INFO "smart_env: 커서 위치 설정: (%d, %d)\n", col, page);
   return 0;
}

// 특정 위치에 텍스트 출력 함수
int ssd1306_print_at(struct i2c_client *client, const char *text, int col, int page)
{
   int ret;

   // 커서 위치 설정
   ret = ssd1306_set_cursor(client, col, page);
   if (ret < 0) return ret;

   // 텍스트 렌더링
   return ssd1306_render_text(client, text, page);
}

// 전체 화면에 여러 줄 텍스트 출력 함수
int ssd1306_print_multiline(struct i2c_client *client, const char *lines[], int num_lines)
{
   int i, ret;

   if (num_lines > 8) {
       printk(KERN_WARNING "smart_env: 최대 8줄까지만 지원 (요청: %d줄)\n", num_lines);
       num_lines = 8;
   }

   for (i = 0; i < num_lines; i++) {
       if (lines[i] != NULL) {
           ret = ssd1306_render_text(client, lines[i], i);
           if (ret < 0) {
               printk(KERN_ERR "smart_env: %d번째 줄 렌더링 실패\n", i);
               return ret;
           }
       }
   }

   printk(KERN_INFO "smart_env: %d줄 멀티라인 텍스트 출력 완료\n", num_lines);
   return 0;
}

// 간단한 그래픽 함수: 수평선 그리기
int ssd1306_draw_hline(struct i2c_client *client, int x, int y, int width)
{
   u8 data[129];
   int page = y / 8;
   int bit_pos = y % 8;
   int i, ret;

   if (page > 7 || x < 0 || x + width > 128) {
       printk(KERN_ERR "smart_env: 잘못된 라인 파라미터\n");
       return -EINVAL;
   }

   // 커서 위치 설정
   ret = ssd1306_set_cursor(client, x, page);
   if (ret < 0) return ret;

   // 라인 데이터 준비
   data[0] = 0x40; // 데이터 모드
   for (i = 1; i <= width; i++) {
       data[i] = (1 << bit_pos);
   }

   ret = i2c_master_send(client, data, width + 1);
   if (ret >= 0) {
       printk(KERN_INFO "smart_env: 수평선 그리기 완료: (%d,%d) 길이=%d\n", x, y, width);
   }

   return ret;
}

// 프레임 테스트 함수 (디버그용)
int ssd1306_test_pattern(struct i2c_client *client)
{
   const char *test_lines[] = {
       "Line 0: TEST",
       "Line 1: HELLO",
       "Line 2: WORLD",
       "Line 3: 12345",
       "Line 4: ABCDE",
       "Line 5: !@#$%",
       "Line 6: .:;?",
       "Line 7: END"
   };

   printk(KERN_INFO "smart_env: 테스트 패턴 출력 시작\n");

   // 화면 지우기
   int ret = ssd1306_clear_display(client);
   if (ret < 0) return ret;

   // 8줄 테스트 텍스트 출력
   return ssd1306_print_multiline(client, test_lines, 8);
}

// 시스템 정보 출력 함수 (실제 사용 예시)
int ssd1306_show_system_info(struct i2c_client *client,
                            const char *temp, const char *humidity,
                            const char *time, const char *status)
{
   const char *info_lines[] = {
       "=== SYSTEM INFO ===",
       temp ? temp : "Temp: N/A",
       humidity ? humidity : "Humidity: N/A",
       time ? time : "Time: N/A",
       "",
       status ? status : "Status: OK",
       "",
       "Press any key..."
   };

   printk(KERN_INFO "smart_env: 시스템 정보 화면 출력\n");

   // 화면 지우기 후 정보 출력
   int ret = ssd1306_clear_display(client);
   if (ret < 0) return ret;

   return ssd1306_print_multiline(client, info_lines, 8);
}
