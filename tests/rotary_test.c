#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>
#include <string.h>

#define GPIO_CHIP_NAME "gpiochip0"
#define CLK_PIN 17
#define DT_PIN 18
#define SW_PIN 27

#define MENU_SIZE 5

const char *menu[MENU_SIZE] = {
    "🍔 햄버거",
    "🍕 피자",
    "🍜 라면",
    "🍣 초밥",
    "🥗 샐러드"
};

int menu_index = 0;

void show_menu() {
    printf("\n📋 메뉴 선택:\n");
    for (int i = 0; i < MENU_SIZE; i++) {
        if (i == menu_index) {
            printf("👉 [%d] %s\n", i, menu[i]);
        } else {
            printf("   [%d] %s\n", i, menu[i]);
        }
    }
}

int main() {
    struct gpiod_chip *chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip) {
        perror("GPIO 칩 열기 실패");
        return 1;
    }

    struct gpiod_line *clk = gpiod_chip_get_line(chip, CLK_PIN);
    struct gpiod_line *dt  = gpiod_chip_get_line(chip, DT_PIN);
    struct gpiod_line *sw  = gpiod_chip_get_line(chip, SW_PIN);

    if (!clk || !dt || !sw) {
        perror("GPIO 라인 가져오기 실패");
        gpiod_chip_close(chip);
        return 1;
    }

    gpiod_line_request_falling_edge_events(clk, "rotary");
    gpiod_line_request_input(dt, "rotary");
    gpiod_line_request_falling_edge_events(sw, "rotary");

    printf("🎛️ 로터리 인코더 메뉴 선택 시작!\n");
    show_menu();

    while (1) {
        struct timespec timeout = {1, 0}; // 1초 대기
        struct gpiod_line_event event;

        if (gpiod_line_event_wait(clk, &timeout) == 1) {
            gpiod_line_event_read(clk, &event);

            int dt_val = gpiod_line_get_value(dt);
            if (dt_val == 0) {
                menu_index++;
            } else {
                menu_index--;
            }

            if (menu_index < 0) menu_index = MENU_SIZE - 1;
            if (menu_index >= MENU_SIZE) menu_index = 0;

            show_menu();
        }

        if (gpiod_line_event_wait(sw, &timeout) == 1) {
            gpiod_line_event_read(sw, &event);
            printf("\n✅ 선택됨: [%d] %s\n", menu_index, menu[menu_index]);
            break;
        }
    }

    gpiod_line_release(clk);
    gpiod_line_release(dt);
    gpiod_line_release(sw);
    gpiod_chip_close(chip);
    return 0;
}
