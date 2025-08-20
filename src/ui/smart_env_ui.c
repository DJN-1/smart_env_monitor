#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <gpiod.h>
#include <signal.h>
#include "smart_env_monitor.h"
#include "dht11_sensor.h"
#include "ds1307_rtc.h"
#include "oled_ioctl.h"
#include "environment_indicator.h"

// 디스플레이 모드 정의
typedef enum {
    DISPLAY_ROOM = 0,      // "LIVING ROOM" + 환경 지수
    DISPLAY_SENSOR = 1,     // 온습도 값
    DISPLAY_TIME = 2,       // 현재 시간
    DISPLAY_MODE_COUNT = 3
} display_mode_t;

// 전역 변수
static display_mode_t current_mode = DISPLAY_ROOM;
static int oled_fd = -1;
static struct gpiod_chip *chip = NULL;
static struct gpiod_line *clk_line = NULL;
static struct gpiod_line *dt_line = NULL;
static struct gpiod_line *sw_line = NULL;
static int last_clk_state = 1;
static volatile int running = 1;
static env_status_t env_status = {0}; // 환경 상태 전역 변수

// 함수 선언
int init_oled_device(void);
int init_rotary_switch(void);
void cleanup_resources(void);
int display_room_name(void);
int display_sensor_data(void);
int display_current_time(void);
int update_display(void);
void handle_rotary_rotation(int direction);
int read_rotary_switch(void);
void signal_handler(int sig);

// 시그널 핸들러 (Ctrl+C 처리)
void signal_handler(int sig) {
    (void)sig; // 경고 제거
    printf("\n🛑 종료 신호 수신. 정리 중...\n");
    running = 0;
}

// OLED 디바이스 초기화
int init_oled_device(void) {
    oled_fd = open("/dev/oled_display", O_RDWR);
    if (oled_fd < 0) {
        perror("❌ OLED 디바이스 열기 실패");
        printf("💡 커널 모듈이 로드되었는지 확인: lsmod | grep oled\n");
        return -1;
    }

    // OLED 초기화
    if (ioctl(oled_fd, OLED_IOC_INIT, 0) < 0) {
        perror("❌ OLED 초기화 실패");
        return -1;
    }

    // 화면 지우기
    if (ioctl(oled_fd, OLED_IOC_CLEAR, 0) < 0) {
        perror("❌ OLED 화면 지우기 실패");
        return -1;
    }

    printf("✅ OLED 디바이스 초기화 완료\n");
    return 0;
}

// 로터리 스위치 초기화
int init_rotary_switch(void) {
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        perror("❌ GPIO 칩 열기 실패");
        return -1;
    }

    clk_line = gpiod_chip_get_line(chip, GPIO_ROTARY_CLK);
    dt_line = gpiod_chip_get_line(chip, GPIO_ROTARY_DT);
    sw_line = gpiod_chip_get_line(chip, GPIO_ROTARY_SW);

    if (!clk_line || !dt_line || !sw_line) {
        perror("❌ GPIO 라인 가져오기 실패");
        return -1;
    }

    // 입력으로 설정
    if (gpiod_line_request_input(clk_line, "rotary_clk") < 0 ||
        gpiod_line_request_input(dt_line, "rotary_dt") < 0 ||
        gpiod_line_request_input(sw_line, "rotary_sw") < 0) {
        perror("❌ GPIO 입력 설정 실패");
        return -1;
    }

    // 초기 CLK 상태 저장
    last_clk_state = gpiod_line_get_value(clk_line);

    printf("✅ 로터리 스위치 초기화 완료\n");
    return 0;
}

// 리소스 정리
void cleanup_resources(void) {
    printf("🧹 리소스 정리 중...\n");

    if (clk_line) {
        gpiod_line_release(clk_line);
        clk_line = NULL;
    }
    if (dt_line) {
        gpiod_line_release(dt_line);
        dt_line = NULL;
    }
    if (sw_line) {
        gpiod_line_release(sw_line);
        sw_line = NULL;
    }
    if (chip) {
        gpiod_chip_close(chip);
        chip = NULL;
    }
    if (oled_fd >= 0) {
        close(oled_fd);
        oled_fd = -1;
    }

    // 센서 정리
    dht11_cleanup();
    ds1307_cleanup();
    gpio_cleanup();

    printf("✅ 리소스 정리 완료\n");
}

// 방 이름과 환경 지수 출력
int display_room_name(void) {
    char command_buffer[128];
    
    // 최신 센서 데이터로 환경 상태 업데이트
    dht11_data_t sensor_data;
    if (dht11_is_ready_to_read() && dht11_read_data(&sensor_data) == 0 && sensor_data.checksum_valid) {
        update_environment_status(&env_status, sensor_data.temperature, sensor_data.humidity);
    } else {
        // 센서 오류 시 기본값
        update_environment_status(&env_status, 22.0f, 50.0f);
    }

    // 방 이름과 환경 지수를 멀티라인으로 표시
    snprintf(command_buffer, sizeof(command_buffer),
            "LIVING ROOM\n%s %s",
            get_level_icon(env_status.overall_level),
            get_level_text(env_status.overall_level));

    // 명령어 전송
    if (write(oled_fd, command_buffer, strlen(command_buffer)) < 0) {
        perror("❌ 방 이름 출력 실패");
        return -1;
    }

    printf("📺 디스플레이 모드 1: 방 이름 + 환경지수 (%s %s)\n", 
           (env_status.overall_level == ENV_GOOD) ? "😊" : 
           (env_status.overall_level == ENV_WARNING) ? "😐" : "😞", 
           get_level_text(env_status.overall_level));
    
    // 콘솔에 상세 정보 표시
    if (env_status.prolonged_warning) {
        printf("⚠️  장기간 주의상태로 인한 위험 승격!\n");
    }
    
    return 0;
}

// 센서 데이터 출력 (멀티라인)
int display_sensor_data(void) {
    dht11_data_t sensor_data;
    char command_buffer[128];

    // DHT11 데이터 읽기
    if (dht11_is_ready_to_read() && dht11_read_data(&sensor_data) == 0 && sensor_data.checksum_valid) {
        // 환경 상태 업데이트
        update_environment_status(&env_status, sensor_data.temperature, sensor_data.humidity);
        
        snprintf(command_buffer, sizeof(command_buffer),
                "T & H\nTEMP: %.1f C\nHUM : %.1f%%",
                sensor_data.temperature,
                sensor_data.humidity);

        printf("📺 디스플레이 모드 2: 센서 데이터 (%.1f°C, %.1f%%)\n",
               sensor_data.temperature, sensor_data.humidity);
    } else {
        snprintf(command_buffer, sizeof(command_buffer), 
                "T & H\nTEMP: ERROR\nHUM : ERROR");
        printf("📺 디스플레이 모드 2: 센서 오류\n");
    }

    // 센서 명령어 전송
    if (write(oled_fd, command_buffer, strlen(command_buffer)) < 0) {
        perror("❌ 센서 데이터 출력 실패");
        return -1;
    }

    return 0;
}

// 현재 시간 출력 (멀티라인)
int display_current_time(void) {
    struct tm current_time;
    char command_buffer[128];

    // RTC에서 시간 읽기 (실패 시 시스템 시간 사용)
    if (ds1307_read_time(&current_time) != 0) {
        time_t now = time(NULL);
        current_time = *localtime(&now);
    }

    snprintf(command_buffer, sizeof(command_buffer),
            "TIME\n%04d-%02d-%02d\n%02d:%02d:%02d",
            current_time.tm_year + 1900,
            current_time.tm_mon + 1,
            current_time.tm_mday,
            current_time.tm_hour,
            current_time.tm_min,
            current_time.tm_sec);

    // 시간 명령어 전송
    if (write(oled_fd, command_buffer, strlen(command_buffer)) < 0) {
        perror("❌ 시간 정보 출력 실패");
        return -1;
    }

    printf("📺 디스플레이 모드 3: 현재 시간 (%04d-%02d-%02d %02d:%02d:%02d)\n",
           current_time.tm_year + 1900, current_time.tm_mon + 1, current_time.tm_mday,
           current_time.tm_hour, current_time.tm_min, current_time.tm_sec);
    return 0;
}

// 현재 모드에 따라 디스플레이 업데이트
int update_display(void) {
    switch (current_mode) {
        case DISPLAY_ROOM:
            return display_room_name();
        case DISPLAY_SENSOR:
            return display_sensor_data();
        case DISPLAY_TIME:
            return display_current_time();
        default:
            return -1;
    }
}

// 로터리 스위치 회전 처리
void handle_rotary_rotation(int direction) {
    if (direction > 0) {
        // 시계방향: 1 -> 2 -> 3 -> 1
        current_mode = (current_mode + 1) % DISPLAY_MODE_COUNT;
        printf("🔄 시계방향 회전: 모드 %d\n", current_mode + 1);
    } else {
        // 반시계방향: 3 -> 2 -> 1 -> 3
        current_mode = (current_mode - 1 + DISPLAY_MODE_COUNT) % DISPLAY_MODE_COUNT;
        printf("🔄 반시계방향 회전: 모드 %d\n", current_mode + 1);
    }

    // 디스플레이 업데이트
    update_display();
}

// 로터리 스위치 상태 읽기
int read_rotary_switch(void) {
    static int last_stable_clk = 1;
    static struct timespec last_change = {0};
    struct timespec now;

    int current_clk = gpiod_line_get_value(clk_line);
    int current_dt = gpiod_line_get_value(dt_line);

    clock_gettime(CLOCK_MONOTONIC, &now);

    // 디바운싱: 10ms 이내의 변화는 무시
    long elapsed_ms = (now.tv_sec - last_change.tv_sec) * 1000 +
                      (now.tv_nsec - last_change.tv_nsec) / 1000000;

    if (elapsed_ms < 10) {
        return 0;
    }

    // CLK 신호의 하강 엣지에서 방향 판별
    if (last_stable_clk == 1 && current_clk == 0) {
        if (current_dt == 0) {
            // 시계방향
            handle_rotary_rotation(1);
        } else {
            // 반시계방향
            handle_rotary_rotation(-1);
        }
        last_change = now;
    }

    last_stable_clk = current_clk;
    return 0;
}

// 메인 함수
int main(void) {
    printf("🚀 Smart Environment Monitor UI 시작\n");
    printf("=====================================\n");

    // 시그널 핸들러 등록
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 환경 상태 초기화
    memset(&env_status, 0, sizeof(env_status));

    // 센서 초기화
    printf("🔧 센서 초기화 중...\n");
    if (gpio_init() != 0) {
        fprintf(stderr, "❌ GPIO 초기화 실패\n");
        return 1;
    }

    if (dht11_init(GPIO_DHT11_DATA) != 0) {
        fprintf(stderr, "❌ DHT11 초기화 실패\n");
        gpio_cleanup();
        return 1;
    }

    if (ds1307_init() != 0) {
        fprintf(stderr, "❌ DS1307 초기화 실패\n");
        dht11_cleanup();
        gpio_cleanup();
        return 1;
    }

    // OLED 디바이스 초기화
    if (init_oled_device() != 0) {
        cleanup_resources();
        return 1;
    }

    // 로터리 스위치 초기화
    if (init_rotary_switch() != 0) {
        cleanup_resources();
        return 1;
    }

    // 초기 화면 표시
    printf("🎯 초기 화면 표시 중...\n");
    if (update_display() != 0) {
        fprintf(stderr, "❌ 초기 화면 표시 실패\n");
        cleanup_resources();
        return 1;
    }

    printf("✅ 모든 초기화 완료!\n");
    printf("=====================================\n");
    printf("🎛️  로터리 스위치로 모드를 전환하세요:\n");
    printf("   모드 1: 방 이름 + 환경지수 (😊=좋음, 😐=주의, 😞=위험)\n");
    printf("   모드 2: 온습도 센서\n");
    printf("   모드 3: 현재 시간\n");
    printf("🔄 회전 방향:\n");
    printf("   시계방향: 1 → 2 → 3 → 1\n");
    printf("   반시계방향: 3 → 2 → 1 → 3\n");
    printf("🌡️  환경 기준:\n");
    printf("   온도 적정: 20~26°C, 주의: 18~28°C\n");
    printf("   습도 적정: 40~60%%, 주의: 30~70%%\n");
    printf("   ⚠️ 주의상태 10초 이상 → 위험으로 승격\n");
    printf("🛑 종료: Ctrl+C\n");
    printf("=====================================\n\n");

    // 메인 루프
    time_t last_update = 0;
    while (running) {
        // 로터리 스위치 상태 확인
        read_rotary_switch();

        // 주기적 업데이트
        time_t now = time(NULL);
        
        if (current_mode == DISPLAY_ROOM && now - last_update >= 3) {  
            // 방 이름 모드: 3초마다 환경지수 업데이트
            update_display();
            last_update = now;
        } else if (current_mode == DISPLAY_SENSOR && now - last_update >= 5) {  
            // 센서 모드: 5초마다 업데이트
            update_display();
            last_update = now;
        } else if (current_mode == DISPLAY_TIME && now - last_update >= 1) {  
            // 시간 모드: 1초마다 업데이트
            update_display();
            last_update = now;
        }

        usleep(50000);  // 50ms 대기 (반응성과 CPU 사용률 균형)
    }

    cleanup_resources();
    printf("👋 프로그램이 정상적으로 종료되었습니다.\n");
    return 0;
}
