#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include "../../include/oled_ioctl.h"
#include "../../include/dht11_sensor.h"
#include "../../include/ds1307_rtc.h"
#include "../../include/gpio_driver.h"
#include "../../include/smart_env_monitor.h"

// 메모리 사용량 체크 함수
void check_memory_usage() {
    FILE *status = fopen("/proc/self/status", "r");
    char line[256];
    
    if (status) {
        while (fgets(line, sizeof(line), status)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                printf("💾 메모리 사용량: %s", line + 6);
                break;
            }
        }
        fclose(status);
    }
}

// 오류 복구 로직
int recovery_attempt(int oled_fd) {
    printf("🔄 오류 복구 시도 중...\n");
    
    // OLED 재초기화
    if (ioctl(oled_fd, OLED_IOC_INIT, 0) < 0) {
        printf("❌ OLED 재초기화 실패\n");
        return -1;
    }
    
    // 화면 지우기
    if (ioctl(oled_fd, OLED_IOC_CLEAR, 0) < 0) {
        printf("❌ 화면 지우기 실패\n");
        return -1;
    }
    
    printf("✅ 복구 완료\n");
    return 0;
}

// 센서 → 커널 드라이버 → OLED 파이프라인
int sensor_to_oled_pipeline(int cycle) {
    int oled_fd = -1;
    dht11_data_t dht_data;
    struct tm rtc_time;
    char display_text[128];
    ssize_t written;
    int error_count = 0;
    
    printf("\n=== 파이프라인 사이클 %d ===\n", cycle);
    
    // 1. OLED 커널 드라이버 열기
    oled_fd = open("/dev/oled_display", O_RDWR);
    if (oled_fd < 0) {
        printf("❌ OLED 디바이스 열기 실패\n");
        return -1;
    }
    printf("📱 OLED 디바이스 연결 완료\n");
    
    // 2. 센서 데이터 수집
    printf("📊 센서 데이터 수집 중...\n");
    
    // DHT11 온습도 데이터
    if (!dht11_is_ready_to_read()) {
        printf("⏳ DHT11 대기 중... (2초 간격)\n");
        sleep(2);
    }
    
    if (dht11_read_data(&dht_data) != 0 || !dht_data.checksum_valid) {
        printf("⚠️ DHT11 데이터 오류 - 기본값 사용\n");
        dht_data.temperature = 25.0f;
        dht_data.humidity = 50.0f;
        error_count++;
    } else {
        printf("🌡️ 온도: %.1f°C, 💧 습도: %.1f%%\n", 
               dht_data.temperature, dht_data.humidity);
    }
    
    // DS1307 시간 데이터
    if (ds1307_read_time(&rtc_time) != 0) {
        printf("⚠️ RTC 데이터 오류 - 시스템 시간 사용\n");
        time_t now = time(NULL);
        rtc_time = *localtime(&now);
        error_count++;
    } else {
        printf("🕐 시간: %02d:%02d:%02d\n", 
               rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
    }
    
    // 3. 데이터 포맷팅
    snprintf(display_text, sizeof(display_text), 
             "T:%.1fC H:%.1f%% %02d:%02d:%02d", 
             dht_data.temperature, dht_data.humidity,
             rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
    
    printf("📝 전송 데이터: %s\n", display_text);
    
    // 4. 커널 드라이버를 통한 OLED 출력
    printf("📤 커널 드라이버로 데이터 전송 중...\n");
    
    // 화면 지우기 (ioctl)
    if (ioctl(oled_fd, OLED_IOC_CLEAR, 0) < 0) {
        printf("❌ 화면 지우기 실패\n");
        error_count++;
        if (recovery_attempt(oled_fd) < 0) {
            close(oled_fd);
            return -1;
        }
    }
    
    // 텍스트 출력 (write 시스템 콜)
    written = write(oled_fd, display_text, strlen(display_text));
    if (written < 0) {
        printf("❌ OLED 쓰기 실패\n");
        error_count++;
        if (recovery_attempt(oled_fd) < 0) {
            close(oled_fd);
            return -1;
        }
    } else {
        printf("✅ OLED 출력 완료 (%zd 바이트)\n", written);
    }
    
    // 5. 성능 및 메모리 체크
    check_memory_usage();
    
    // 6. 정리
    close(oled_fd);
    
    printf("⚠️ 이번 사이클 오류 수: %d\n", error_count);
    return error_count;
}

int main() {
    int total_errors = 0;
    int failed_cycles = 0;
    
    printf("=== Day 3 최종: 센서 → 커널 드라이버 → OLED 통합 파이프라인 ===\n");
    
    // 1. 초기화
    printf("\n🔧 시스템 초기화 중...\n");
    
    if (gpio_init() != 0) {
        printf("❌ GPIO 초기화 실패\n");
        return 1;
    }
    printf("✅ GPIO 초기화 완료\n");
    
    if (dht11_init(GPIO_DHT11_DATA) != 0) {
        printf("❌ DHT11 초기화 실패\n");
        return 1;
    }
    printf("✅ DHT11 초기화 완료\n");
    
    if (ds1307_init() != 0) {
        printf("❌ DS1307 초기화 실패\n");
        return 1;
    }
    printf("✅ DS1307 초기화 완료\n");
    
    // 2. 파이프라인 반복 테스트 (10회)
    printf("\n🔄 파이프라인 스트레스 테스트 시작 (10 사이클)\n");
    
    for (int i = 1; i <= 10; i++) {
        int cycle_errors = sensor_to_oled_pipeline(i);
        
        if (cycle_errors < 0) {
            printf("💥 사이클 %d 치명적 실패!\n", i);
            failed_cycles++;
        } else {
            total_errors += cycle_errors;
        }
        
        // 3초 대기 (DHT11 간격 + 안정화)
        printf("⏳ 다음 사이클까지 3초 대기...\n");
        sleep(3);
    }
    
    // 3. 최종 결과 분석
    printf("\n==================================================\n");
    printf("📊 최종 통합 테스트 결과:\n");
    printf("   🔄 총 사이클: 10\n");
    printf("   💥 실패한 사이클: %d\n", failed_cycles);
    printf("   ⚠️ 총 오류 수: %d\n", total_errors);
    printf("   ✅ 성공률: %.1f%%\n", ((10.0 - failed_cycles) / 10.0) * 100);
    
    if (failed_cycles == 0 && total_errors < 5) {
        printf("\n🎉 Day 3 통합 파이프라인 성공! Day 4로 진행 가능!\n");
    } else if (failed_cycles < 3) {
        printf("\n⚠️ 일부 문제 있음. 최적화 후 Day 4 진행 권장\n");
    } else {
        printf("\n❌ 심각한 문제 발견. 디버깅 필요\n");
    }
    
    // 4. 정리
    ds1307_cleanup();
    gpio_cleanup();
    
    return (failed_cycles > 3) ? 1 : 0;
}
