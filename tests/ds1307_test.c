#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define I2C_DEVICE "/dev/i2c-1"
#define DS1307_I2C_ADDR 0x68

// BCD (Binary-Coded Decimal)를 10진수로 변환
unsigned char bcdToDec(unsigned char val) {
    return (val / 16 * 10) + (val % 16);
}

int main() {
    int fd;
    unsigned char reg = 0x00;  // 읽기 시작 레지스터 주소
    unsigned char data[7];     // Seconds, Minutes, Hours, Day, Date, Month, Year
    
    printf("=== DS1307 RTC 시간 읽기 테스트 ===\n");
    
    fd = open(I2C_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("I2C 디바이스 열기 실패");
        return 1;
    }
    printf("I2C 디바이스 열기 성공\n");
    
    if (ioctl(fd, I2C_SLAVE, DS1307_I2C_ADDR) < 0) {
        perror("DS1307 슬레이브 설정 실패");
        close(fd);
        return 1;
    }
    printf("DS1307 I2C 통신 성공 (주소: 0x%02X)\n", DS1307_I2C_ADDR);
    
    // 먼저 읽기 시작 레지스터 주소(0x00)를 써야 함
    if (write(fd, &reg, 1) != 1) {
        perror("DS1307 레지스터 주소 설정 실패");
        close(fd);
        return 1;
    }
    
    // 0x00 레지스터부터 7바이트 읽기
    if (read(fd, data, sizeof(data)) != sizeof(data)) {
        perror("DS1307 데이터 읽기 실패");
        close(fd);
        return 1;
    }
    printf("DS1307 데이터 읽기 성공\n");
    
    printf("\nDS1307 현재 시간:\n");
    printf("  년: 20%02d\n", bcdToDec(data[6]));
    printf("  월: %02d\n", bcdToDec(data[5] & 0x1F));
    printf("  일: %02d\n", bcdToDec(data[4] & 0x3F));
    printf("  시: %02d\n", bcdToDec(data[2] & 0x3F));
    printf("  분: %02d\n", bcdToDec(data[1] & 0x7F));
    printf("  초: %02d\n", bcdToDec(data[0] & 0x7F));
    
    close(fd);
    printf("DS1307 테스트 완료!\n");
    return 0;
}
