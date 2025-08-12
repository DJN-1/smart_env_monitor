#ifndef DS1307_RTC_H
#define DS1307_RTC_H

#include "smart_env_monitor.h"

// DS1307 I2C 주소
#define DS1307_I2C_ADDR       0x68

// DS1307 레지스터 주소
#define DS1307_REG_SECONDS    0x00
#define DS1307_REG_MINUTES    0x01
#define DS1307_REG_HOURS      0x02
#define DS1307_REG_DAY        0x03
#define DS1307_REG_DATE       0x04
#define DS1307_REG_MONTH      0x05
#define DS1307_REG_YEAR       0x06
#define DS1307_REG_CONTROL    0x07

// DS1307 제어 비트
#define DS1307_CLOCK_HALT     0x80  // CH bit in seconds register
#define DS1307_12_HOUR_MODE   0x40  // 12/24 hour mode bit
#define DS1307_PM_BIT         0x20  // PM bit in 12-hour mode

// BCD 변환 매크로
#define BCD_TO_DEC(val)  (((val) >> 4) * 10 + ((val) & 0x0F))
#define DEC_TO_BCD(val)  ((((val) / 10) << 4) + ((val) % 10))

// DS1307 RTC 함수
int ds1307_init(void);
void ds1307_cleanup(void);
int ds1307_read_time(struct tm *time);
int ds1307_write_time(const struct tm *time);
int ds1307_set_current_time(void);
void ds1307_print_time(const struct tm *time);
int ds1307_start_clock(void);
int ds1307_stop_clock(void);

// 내부 I2C 통신 함수
static int ds1307_read_register(unsigned char reg);
static int ds1307_write_register(unsigned char reg, unsigned char data);
static int ds1307_read_burst(unsigned char *data, int len);
static int ds1307_write_burst(const unsigned char *data, int len);

#endif // DS1307_RTC_H
