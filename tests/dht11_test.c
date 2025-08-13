#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <stdint.h>

#define DHT11_PIN 4
#define GPIO_CHIP_NAME "gpiochip0"

struct gpiod_chip *chip = NULL;
struct gpiod_line *line = NULL;

// 실시간 스케줄링 적용
void enable_realtime() {
    struct sched_param param = { .sched_priority = 99 };
    sched_setscheduler(0, SCHED_FIFO, &param);
}

// 마이크로초 단위 시간 측정
uint64_t get_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

// 상태 변화 대기 및 경과 시간 반환
int wait_for_state(int expected_state, int timeout_us) {
    uint64_t start = get_time_us();
    while (get_time_us() - start < timeout_us) {
        if (gpiod_line_get_value(line) == expected_state) {
            return get_time_us() - start;
        }
    }
    return -1;
}

// DHT11 데이터 읽기
int read_dht11() {
    unsigned char data[5] = {0};

    printf("🔄 DHT11 통신 시작 (정밀 타이밍)...\n");

    // 1. 시작 신호: LOW 18ms
    gpiod_line_request_output(line, "dht11", 0);
    usleep(18000);

    // 2. 릴리즈: HIGH 30us
    gpiod_line_set_value(line, 1);
    usleep(30);

    // 3. 입력 모드 전환
    gpiod_line_release(line);
    gpiod_line_request_input(line, "dht11");

    // 4. 응답 신호 대기 (시간 증가)
    if (wait_for_state(0, 200) < 0) {
        printf("❌ DHT11 응답 LOW 대기 실패\n");
        return -1;
    }
    printf("✅ DHT11 응답 LOW\n");

    if (wait_for_state(1, 200) < 0) {
        printf("❌ DHT11 응답 HIGH 대기 실패\n");
        return -1;
    }
    printf("✅ DHT11 응답 HIGH\n");

    // 5. 40비트 데이터 수신
    for (int i = 0; i < 40; i++) {
        if (wait_for_state(0, 100) < 0) {
            printf("❌ 비트 %d: LOW 신호 대기 실패\n", i);
            return -1;
        }

        if (wait_for_state(1, 100) < 0) {
            printf("❌ 비트 %d: HIGH 신호 대기 실패\n", i);
            return -1;
        }

        uint64_t start = get_time_us();
        while (gpiod_line_get_value(line) == 1) {
            if (get_time_us() - start > 100) break;
        }
        int high_time = get_time_us() - start;

        data[i / 8] <<= 1;
        if (high_time > 45) data[i / 8] |= 1;  // ✅ 기준 완화

        if (i < 8) {
            printf("비트 %d: HIGH=%dus, 값=%d\n", i, high_time, (high_time > 45) ? 1 : 0);
        }
    }

    // 6. 결과 출력
    printf("\n📊 수신 데이터:\n");
    for (int i = 0; i < 5; i++) {
        printf("바이트 %d: 0x%02X (%d)\n", i, data[i], data[i]);
    }

    unsigned char checksum = data[0] + data[1] + data[2] + data[3];
    printf("체크섬: 계산=0x%02X, 수신=0x%02X\n", checksum, data[4]);

    if (checksum == data[4]) {
        printf("✅ 체크섬 검증 성공!\n");
        printf("🌡️  온도: %d.%d°C\n", data[2], data[3]);
        printf("💧 습도: %d.%d%%\n", data[0], data[1]);
        return 0;
    } else {
        printf("❌ 체크섬 검증 실패\n");
        printf("🔍 예상 바이트: H=%d.%d, T=%d.%d\n", data[0], data[1], data[2], data[3]);
        return -1;
    }
}

int main() {
    printf("=== DHT11 정밀 테스트 시작 ===\n");

    enable_realtime(); // 실시간 스케줄링 적용

    chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip) {
        perror("GPIO 칩 열기 실패");
        return 1;
    }

    line = gpiod_chip_get_line(chip, DHT11_PIN);
    if (!line) {
        perror("GPIO 라인 가져오기 실패");
        gpiod_chip_close(chip);
        return 1;
    }

    for (int i = 1; i <= 3; i++) {
        printf("\n🔄 시도 %d/3\n", i);
        if (read_dht11() == 0) {
            printf("🎉 성공!\n");
            break;
        }
        if (i < 3) {
            printf("⏳ 5초 대기...\n");
            sleep(5);  // ✅ 안정화 시간 증가
        }
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
