# 라즈베리파이 4 핀맵 설계

## GPIO 핀 할당

### I2C (OLED SSD1306 + DS1307 RTC)
- SDA: GPIO 2 (Pin 3) - 공유
- SCL: GPIO 3 (Pin 5) - 공유
- OLED VCC: 3.3V (Pin 1)
- OLED GND: GND (Pin 6)
- DS1307 VCC: 3.3V (Pin 17)
- DS1307 GND: GND (Pin 20)

### 1-Wire (DHT11)
- DATA: GPIO 4 (Pin 7)
- VCC: 5V (Pin 2)
- GND: GND (Pin 9)
- 풀업 저항: 10kΩ (VCC-DATA 간)

### GPIO (로터리 스위치)
- CLK: GPIO 17 (Pin 11)
- DT: GPIO 18 (Pin 12)
- SW: GPIO 27 (Pin 13)
- VCC: 3.3V (Pin 1)
- GND: GND (Pin 14)

### UART (외부 통신)
- TXD: GPIO 14 (Pin 8)
- RXD: GPIO 15 (Pin 10)
- GND: GND (Pin 6) - 공유

## 핀맵 요약
- 사용 GPIO: 2, 3, 4, 14, 15, 17, 18, 27
- 전원: 3.3V (Pin 1, 17), 5V (Pin 2)
- GND: Pin 6, 9, 14, 20

## I2C 주소 할당
- OLED SSD1306: 0x3C (또는 0x3D)
- DS1307 RTC: 0x68
