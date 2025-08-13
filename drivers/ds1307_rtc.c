#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include "ds1307_rtc.h"

static int i2c_fd = -1;

int ds1307_init(void) {
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        perror("I2C 디바이스 열기 실패");
        return -1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, DS1307_I2C_ADDR) < 0) {
        perror("DS1307 슬레이브 설정 실패");
        close(i2c_fd);
        i2c_fd = -1;
        return -1;
    }

    return 0;
}

void ds1307_cleanup(void) {
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
}

static int ds1307_read_register(unsigned char reg) {
    if (write(i2c_fd, &reg, 1) != 1) return -1;
    unsigned char val;
    if (read(i2c_fd, &val, 1) != 1) return -1;
    return val;
}

static int ds1307_write_register(unsigned char reg, unsigned char data) {
    unsigned char buf[2] = {reg, data};
    return (write(i2c_fd, buf, 2) == 2) ? 0 : -1;
}

static int ds1307_read_burst(unsigned char *data, int len) {
    unsigned char reg = DS1307_REG_SECONDS;
    if (write(i2c_fd, &reg, 1) != 1) return -1;
    return (read(i2c_fd, data, len) == len) ? 0 : -1;
}

int ds1307_write_burst(const unsigned char *data, int len) {
    unsigned char buf[len + 1];
    buf[0] = DS1307_REG_SECONDS;
    for (int i = 0; i < len; i++) {
        buf[i + 1] = data[i];
    }
    return (write(i2c_fd, buf, len + 1) == len + 1) ? 0 : -1;
}

int ds1307_read_time(struct tm *time) {
    unsigned char data[7];
    if (ds1307_read_burst(data, 7) != 0) return -1;

    time->tm_sec  = BCD_TO_DEC(data[0] & 0x7F);
    time->tm_min  = BCD_TO_DEC(data[1]);
    time->tm_hour = BCD_TO_DEC(data[2] & 0x3F);  // 24시간 모드 기준
    time->tm_wday = BCD_TO_DEC(data[3]) - 1;     // tm_wday: 0=일요일
    time->tm_mday = BCD_TO_DEC(data[4]);
    time->tm_mon  = BCD_TO_DEC(data[5]) - 1;     // tm_mon: 0=1월
    time->tm_year = BCD_TO_DEC(data[6]) + 100;   // tm_year: 1900 기준

    return 0;
}

int ds1307_write_time(const struct tm *time) {
    unsigned char data[7];
    data[0] = DEC_TO_BCD(time->tm_sec) & 0x7F;
    data[1] = DEC_TO_BCD(time->tm_min);
    data[2] = DEC_TO_BCD(time->tm_hour);  // 24시간 모드
    data[3] = DEC_TO_BCD(time->tm_wday + 1);
    data[4] = DEC_TO_BCD(time->tm_mday);
    data[5] = DEC_TO_BCD(time->tm_mon + 1);
    data[6] = DEC_TO_BCD(time->tm_year - 100);

    return ds1307_write_burst(data, 7);
}

int ds1307_set_current_time(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    return ds1307_write_time(local);
}

void ds1307_print_time(const struct tm *time) {
    printf("🕒 %04d-%02d-%02d %02d:%02d:%02d\n",
           time->tm_year + 1900,
           time->tm_mon + 1,
           time->tm_mday,
           time->tm_hour,
           time->tm_min,
           time->tm_sec);
}

int ds1307_start_clock(void) {
    int sec = ds1307_read_register(DS1307_REG_SECONDS);
    if (sec < 0) return -1;
    return ds1307_write_register(DS1307_REG_SECONDS, sec & ~DS1307_CLOCK_HALT);
}

int ds1307_stop_clock(void) {
    int sec = ds1307_read_register(DS1307_REG_SECONDS);
    if (sec < 0) return -1;
    return ds1307_write_register(DS1307_REG_SECONDS, sec | DS1307_CLOCK_HALT);
}
