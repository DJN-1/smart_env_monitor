#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <time.h>

#define I2C_DEVICE "/dev/i2c-1"
#define DS1307_I2C_ADDR 0x68

// 10진수를 BCD로 변환
unsigned char decToBcd(int val) {
    return (val / 10 * 16) + (val % 10);
}

int main() {
    int fd;
    time_t now;
    struct tm *timeinfo;
    unsigned char data[8];  // 레지스터 주소 + 7바이트 시간 데이터
    
    printf("=== DS1307 현재 시간 설정 ===\n");
    
    // 현재 시간 가져오기
    time(&now);
    timeinfo = localtime(&now);
    
    printf("설정할 시간: %04d-%02d-%02d %02d:%02d:%02d\n",
           timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    fd = open(I2C_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("I2C 디바이스 열기 실패");
        return 1;
    }
    
    if (ioctl(fd, I2C_SLAVE, DS1307_I2C_ADDR) < 0) {
        perror("DS1307 슬레이브 설정 실패");
        close(fd);
        return 1;
    }
    
    // DS1307에 쓸 데이터 준비
    data[0] = 0x00;  // 시작 레지스터 주소
    data[1] = decToBcd(timeinfo->tm_sec);   // 초 (CH=0, 클럭 시작)
    data[2] = decToBcd(timeinfo->tm_min);   // 분
    data[3] = decToBcd(timeinfo->tm_hour);  // 시 (24시간 모드)
    data[4] = decToBcd(timeinfo->tm_wday + 1); // 요일 (1-7)
    data[5] = decToBcd(timeinfo->tm_mday);  // 일
    data[6] = decToBcd(timeinfo->tm_mon + 1);  // 월 (1-12)
    data[7] = decToBcd(timeinfo->tm_year % 100); // 년 (00-99)
    
    // DS1307에 시간 데이터 쓰기
    if (write(fd, data, 8) != 8) {
        perror("DS1307 시간 설정 실패");
        close(fd);
        return 1;
    }
    
    printf("✅ DS1307 시간 설정 완료!\n");
    printf("3초 후 설정된 시간을 확인합니다...\n");
    
    close(fd);
    sleep(3);
    
    // 설정된 시간 확인
    system("./ds1307_test");
    
    return 0;
}
