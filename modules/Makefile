KERNEL_VERSION := $(shell uname -r)
KERNEL_DIR := /lib/modules/$(KERNEL_VERSION)/build

# 헤더 경로 추가
EXTRA_CFLAGS += -I$(PWD)/../include

# 커널 모듈들
obj-m += hello_module.o
obj-m += oled_driver.o

# oled_driver는 여러 소스 파일로 구성
oled_driver-objs := oled_i2c_driver.o oled_ssd1306_commands.o

all:
	@echo "=== 커널 모듈 빌드 시작 ==="
	@echo "커널 버전: $(KERNEL_VERSION)"
	@echo "헤더 경로: $(PWD)/../include"
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean

# 테스트 프로그램 빌드
test-app:
	@echo "=== 테스트 프로그램 빌드 ==="
	cd ../tests/day3 && gcc -o oled_test oled_test.c -I../../include
	chmod +x ../tests/day3/oled_test

# 설치 및 테스트
install-hello:
	sudo insmod hello_module.ko

install-oled:
	@if lsmod | grep -q oled_driver; then \
		echo "모듈이 이미 로드됨. 제거 후 재삽입."; \
		sudo rmmod oled_driver; \
	fi
	sudo insmod oled_driver.ko
	@echo "=== I2C 디바이스 바인딩 ==="
	@if [ ! -e /sys/bus/i2c/devices/i2c-1/1-003c ]; then \
		echo "ssd1306 0x3c" | sudo tee /sys/bus/i2c/devices/i2c-1/new_device; \
	else \
		echo "⚠️ 이미 바인딩된 디바이스입니다. 생략합니다."; \
	fi
	@echo "=== 디바이스 파일 확인 ==="
	ls -la /dev/oled_display
	@echo "=== 디바이스 파일 권한 설정 ==="
	sudo chmod 666 /dev/oled_display

remove-hello:
	sudo rmmod hello_module

remove-oled:
	sudo rmmod oled_driver

# 종합 테스트
test: all test-app install-oled
	@echo "=== 종합 테스트 시작 ==="
	../tests/day3/oled_test "Hello from Kernel!"
	@echo "=== 커널 로그 확인 ==="
	dmesg | tail -10

log:
	dmesg | tail -15

info:
	@echo "=== 모듈 정보 ==="
	lsmod | grep -E "(hello|oled)"
	@echo "=== I2C 디바이스 ==="
	ls /sys/bus/i2c/devices/ | grep 1-003c
	@echo "=== 디바이스 파일 ==="
	ls -la /dev/oled_display 2>/dev/null || echo "디바이스 파일 없음"

.PHONY: all clean test-app install-hello install-oled remove-hello remove-oled test log info
