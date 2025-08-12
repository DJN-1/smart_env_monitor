#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <gpiod.h>

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    
    printf("=== libgpiod 기본 테스트 ===\n");
    
    // GPIO 칩 열기
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        printf("❌ GPIO 칩 열기 실패\n");
        return 1;
    }
    printf("✅ GPIO 칩 열기 성공\n");
    
    // GPIO 18 라인 가져오기
    line = gpiod_chip_get_line(chip, 18);
    if (!line) {
        printf("❌ GPIO 18 라인 가져오기 실패\n");
        gpiod_chip_close(chip);
        return 1;
    }
    printf("✅ GPIO 18 라인 가져오기 성공\n");
    
    // 출력으로 설정
    if (gpiod_line_request_output(line, "test", 0) < 0) {
        printf("❌ GPIO 18 출력 설정 실패\n");
        gpiod_chip_close(chip);
        return 1;
    }
    printf("✅ GPIO 18 출력 설정 성공\n");
    
    // LED 깜빡임 테스트 (GPIO 18에 LED 연결하면 보임)
    printf("🔄 LED 깜빡임 테스트 시작 (GPIO 18)\n");
    for (int i = 0; i < 5; i++) {
        gpiod_line_set_value(line, 1);
        printf("   💡 LED ON (%d/5)\n", i+1);
        usleep(500000);
        
        gpiod_line_set_value(line, 0);
        printf("   ⚫ LED OFF (%d/5)\n", i+1);
        usleep(500000);
    }
    
    // 정리
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    printf("✅ 모든 테스트 완료!\n");
    
    return 0;
}
