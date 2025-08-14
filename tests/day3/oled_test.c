#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "../include/oled_ioctl.h"

int main(int argc, char *argv[])
{
    int fd;
    int ret;
    
    printf("=== OLED 테스트 프로그램 ===\n");
    
    // 디바이스 파일 열기
    fd = open("/dev/oled_display", O_RDWR);
    if (fd < 0) {
        perror("❌ 디바이스 열기 실패");
        printf("💡 드라이버가 로드되었는지 확인하세요: lsmod | grep oled\n");
        return -1;
    }
    printf("✅ /dev/oled_display 열기 성공\n");
    
    // OLED 초기화
    printf("🔄 OLED 초기화 중...\n");
    ret = ioctl(fd, OLED_IOC_INIT, 0);
    if (ret < 0) {
        perror("❌ OLED 초기화 실패");
        close(fd);
        return -1;
    }
    printf("✅ OLED 초기화 완료\n");
    
    // 화면 지우기
    printf("🧹 화면 지우기...\n");
    ret = ioctl(fd, OLED_IOC_CLEAR, 0);
    if (ret < 0) {
        perror("❌ 화면 지우기 실패");
        close(fd);
        return -1;
    }
    printf("✅ 화면 지우기 완료\n");
    
    // 사용자 메시지 처리
    if (argc > 1) {
        printf("📝 메시지 전송: '%s'\n", argv[1]);
        ssize_t written = write(fd, argv[1], strlen(argv[1]));
        if (written < 0) {
            perror("❌ 메시지 전송 실패");
        } else {
            printf("✅ %zd 바이트 전송 완료\n", written);
        }
    }
    
    // 대비 테스트 (선택사항)
    if (argc > 2) {
        int contrast = atoi(argv[2]);
        if (contrast >= 0 && contrast <= 255) {
            printf("🎨 대비 설정: %d\n", contrast);
            ret = ioctl(fd, OLED_IOC_CONTRAST, contrast);
            if (ret < 0) {
                perror("❌ 대비 설정 실패");
            } else {
                printf("✅ 대비 설정 완료\n");
            }
        }
    }
    
    printf("🎯 테스트 완료!\n");
    printf("💡 커널 로그 확인: dmesg | tail -10\n");
    
    close(fd);
    return 0;
}
