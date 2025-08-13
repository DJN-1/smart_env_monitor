#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEVICE "/dev/i2c-1"
#define OLED_ADDR 0x3C

int main() {
    int fd;
    
    printf("=== OLED SSD1306 I2C 테스트 ===\n");
    
    // I2C 디바이스 열기
    fd = open(I2C_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("❌ I2C 디바이스 열기 실패");
        return 1;
    }
    printf("✅ I2C 디바이스 열기 성공\n");
    
    // OLED 슬레이브 설정
    if (ioctl(fd, I2C_SLAVE, OLED_ADDR) < 0) {
        printf("❌ OLED 슬레이브 설정 실패\n");
        close(fd);
        return 1;
    }
    printf("✅ OLED I2C 통신 성공 (주소: 0x%02X)\n", OLED_ADDR);
    
    // OLED 초기화 및 화면 켜기
    printf("🔄 OLED 화면 초기화 중...\n");
    unsigned char oled_init[] = {
        0x00, 0xAE, // Display OFF
        0x00, 0xD5, 0x00, 0x80, // Set display clock
        0x00, 0xA8, 0x00, 0x3F, // Set multiplex ratio (64-1)
        0x00, 0xD3, 0x00, 0x00, // Set display offset
        0x00, 0x40, // Set start line
        0x00, 0x8D, 0x00, 0x14, // Charge pump enable
        0x00, 0x20, 0x00, 0x00, // Memory addressing mode
        0x00, 0xA1, // Segment remap
        0x00, 0xC8, // COM output scan direction
        0x00, 0xAF  // Display ON
    };
    
    for (int i = 0; i < sizeof(oled_init); i += 2) {
        if (write(fd, &oled_init[i], 2) != 2) {
            printf("❌ OLED 초기화 실패\n");
            close(fd);
            return 1;
        }
        usleep(1000); // 1ms 지연
    }
    
    printf("✅ OLED 초기화 완료!\n");
    printf("💡 OLED 화면이 약간 밝아졌는지 확인하세요.\n");
    printf("   (검은 화면에서 약간 회색빛으로 변해야 합니다)\n");
    
    close(fd);
    return 0;
}
