#include <stdio.h>
#include <time.h>
#include "ds1307_rtc.h"

int main(void) {
    // RTC 초기화
    if (ds1307_init() != 0) {
        fprintf(stderr, "RTC 초기화 실패\n");
        return 1;
    }

    // 현재 시스템 시간으로 RTC 설정
    if (ds1307_set_current_time() != 0) {
        fprintf(stderr, "RTC 시간 설정 실패\n");
        ds1307_cleanup();
        return 1;
    }

    // RTC에서 시간 읽기
    struct tm rtc_time;
    if (ds1307_read_time(&rtc_time) != 0) {
        fprintf(stderr, "RTC 시간 읽기 실패\n");
        ds1307_cleanup();
        return 1;
    }

    // 시간 출력
    ds1307_print_time(&rtc_time);

    // RTC 클럭 정지 테스트
    if (ds1307_stop_clock() != 0) {
        fprintf(stderr, "RTC 클럭 정지 실패\n");
    } else {
        printf("✅ RTC 클럭 정지 완료\n");
    }

    // RTC 클럭 시작 테스트
    if (ds1307_start_clock() != 0) {
        fprintf(stderr, "RTC 클럭 시작 실패\n");
    } else {
        printf("✅ RTC 클럭 시작 완료\n");
    }

    // 종료
    ds1307_cleanup();
    return 0;
}
