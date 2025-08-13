#include <stdio.h>
#include <time.h>
#include "ds1307_rtc.h"

int main(void) {
    if (ds1307_init() != 0) {
        fprintf(stderr, "I2C 초기화 실패\n");
        return 1;
    }

    // 초 레지스터 읽기
    int sec_raw = ds1307_read_register(DS1307_REG_SECONDS);
    if (sec_raw < 0) {
        fprintf(stderr, "초 레지스터 읽기 실패\n");
    } else {
        int sec = BCD_TO_DEC(sec_raw & 0x7F);
        printf("✅ 초 레지스터 값: 0x%02X → %d초\n", sec_raw, sec);
    }

    // 초 레지스터에 30초 설정
    int new_sec = DEC_TO_BCD(30) & 0x7F;
    if (ds1307_write_register(DS1307_REG_SECONDS, new_sec) != 0) {
        fprintf(stderr, "초 레지스터 쓰기 실패\n");
    } else {
        printf("✅ 초 레지스터에 30초 설정 완료\n");
    }

    // 다시 읽어서 확인
    sec_raw = ds1307_read_register(DS1307_REG_SECONDS);
    if (sec_raw >= 0) {
        int sec = BCD_TO_DEC(sec_raw & 0x7F);
        printf("🔄 확인된 초 값: 0x%02X → %d초\n", sec_raw, sec);
    }

    ds1307_cleanup();
    return 0;
}
