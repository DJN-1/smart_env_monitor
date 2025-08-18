KERNEL_VERSION := $(shell uname -r)
KERNEL_DIR := /lib/modules/$(KERNEL_VERSION)/build

# 헤더 경로 추가
EXTRA_CFLAGS += -I$(PWD)/../include

# 커널 모듈들
obj-m += hello_module.o
obj-m += oled_driver.o

# oled_driver 구성 (같은 폴더 + 외부 라이브러리)
oled_driver-objs := oled_i2c_driver.o ../src/display/oled_ssd1306_commands.o

all:
	@echo "=== 커널 모듈 빌드 시작 ==="
	@echo "빌드 위치: $(PWD)"
	@echo "커널 버전: $(KERNEL_VERSION)"
	@echo "소스 파일들:"
	@ls -la *.c 2>/dev/null || echo "   ❌ .c 파일이 없습니다!"
	@echo "디스플레이 라이브러리: ../src/display/oled_ssd1306_commands.c"
	@ls -la ../src/display/oled_ssd1306_commands.c 2>/dev/null || echo "   ❌ 라이브러리가 없습니다!"
	
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	
	@echo "=== 빌드 결과물을 modules 폴더로 복사 ==="
	mkdir -p ../modules
	cp *.ko ../modules/ 2>/dev/null || true
	@echo "✅ 빌드 완료!"

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	rm -f ../modules/*.ko 2>/dev/null || true
	find ../src/display -name "*.o" -delete 2>/dev/null || true

# 테스트 프로그램 빌드
test-app:
	@echo "=== 테스트 프로그램 빌드 ==="
	cd ../tests/day3 && gcc -o oled_test oled_test.c -I../../include
	chmod +x ../tests/day3/oled_test

# 설치 및 테스트
install-oled: all
	@if lsmod | grep -q oled_driver; then \
		echo "기존 모듈 제거 중..."; \
		sudo rmmod oled_driver; \
	fi
	sudo insmod ../modules/oled_driver.ko
	@echo "=== I2C 디바이스 바인딩 ==="
	@if [ ! -e /sys/bus/i2c/devices/i2c-1/1-003c ]; then \
		echo "ssd1306 0x3c" | sudo tee /sys/bus/i2c/devices/i2c-1/new_device; \
	else \
		echo "⚠️ 이미 바인딩된 디바이스입니다."; \
	fi
	sudo chmod 666 /dev/oled_display 2>/dev/null || true
	@echo "✅ OLED 드라이버 설치 완료!"

remove-oled:
	sudo rmmod oled_driver

test: all test-app install-oled
	@echo "=== 종합 테스트 시작 ==="
	../tests/day3/oled_test "Hello from drivers folder!"
	@echo "=== 커널 로그 확인 ==="
	dmesg | tail -10

check:
	@echo "=== 파일 구조 점검 ==="
	@echo "📁 현재 위치: $(PWD)"
	@echo "📋 현재 폴더 .c 파일들:"
	@ls -la *.c 2>/dev/null || echo "   ❌ .c 파일이 없습니다!"
	@echo "📋 디스플레이 라이브러리:"
	@ls -la ../src/display/*.c 2>/dev/null || echo "   ❌ src/display/*.c 없음"
	@echo "📋 헤더 파일들:"
	@ls -la ../include/*.h 2>/dev/null || echo "   ❌ include/*.h 없음"
	@echo "📋 테스트 프로그램:"
	@ls -la ../tests/day3/oled_test* 2>/dev/null || echo "   ❌ tests/day3/oled_test 없음"

info:
	@echo "=== 빌드 환경 정보 ==="
	@echo "빌드 위치: $(PWD)"
	@echo "결과물 위치: $(PWD)/../modules"
	@echo "테스트 위치: $(PWD)/../tests/day3"
	@echo ""
	@echo "=== 로드된 모듈 ==="
	lsmod | grep -E "(hello|oled)" || echo "로드된 모듈 없음"

.PHONY: all clean test-app install-oled remove-oled test check info
