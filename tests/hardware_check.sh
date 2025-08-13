#!/bin/bash

if [ "$EUID" -ne 0 ]; then
    echo "🔐 루트 권한이 필요합니다. sudo로 재실행합니다..."
    exec sudo "$0" "$@"
fi

echo "=== 하드웨어 연결 종합 점검 ==="

echo "1. I2C 디바이스 검색..."
i2cdetect -y 1

echo -e "\n2. SPI 디바이스 확인..."
ls -l /dev/spi* 2>/dev/null || echo "SPI 디바이스 없음"

echo -e "\n3. GPIO 상태 확인..."
gpio readall 2>/dev/null || echo "WiringPi GPIO 확인 불가"

echo -e "\n4. 커널 모듈 로드 상태..."
lsmod | grep -E "(i2c|spi|gpio)"

echo -e "\n5. 디바이스 트리 확인..."
find /proc/device-tree/ -type d | grep -iE "(i2c|spi|gpio)" || echo "관련 디바이스 트리 항목 없음"

echo -e "\n=== 점검 완료 ==="
