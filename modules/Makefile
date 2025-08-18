# 📦 modules 폴더: 빌드 결과물(.ko 파일들)만 저장
# 🔨 실제 빌드는 drivers 폴더에서 수행

all:
	@echo "⚠️  빌드는 drivers 폴더에서 수행하세요!"
	@echo "    cd ../drivers && make all"
	@echo ""
	@echo "📁 현재 빌드된 모듈들:"
	@ls -la *.ko 2>/dev/null || echo "    빌드된 모듈이 없습니다"

clean:
	@echo "🧹 빌드 결과물 정리 중..."
	rm -f *.ko *.o *.mod *.mod.c *.symvers *.order
	@echo "✅ 정리 완료!"

# 빌드된 모듈 설치 (빌드는 하지 않음)
install-oled:
	@if [ -f oled_driver.ko ]; then \
		echo "📦 OLED 드라이버 설치 중..."; \
		if lsmod | grep -q oled_driver; then \
			echo "기존 모듈 제거 중..."; \
			sudo rmmod oled_driver; \
		fi; \
		sudo insmod oled_driver.ko; \
		echo "ssd1306 0x3c" | sudo tee /sys/bus/i2c/devices/i2c-1/new_device 2>/dev/null || true; \
		sudo chmod 666 /dev/oled_display 2>/dev/null || true; \
		echo "✅ OLED 드라이버 설치 완료!"; \
	else \
		echo "❌ oled_driver.ko가 없습니다!"; \
		echo "   먼저 빌드하세요: cd ../drivers && make all"; \
	fi

install-hello:
	@if [ -f hello_module.ko ]; then \
		sudo insmod hello_module.ko; \
		echo "✅ Hello 모듈 설치 완료!"; \
	else \
		echo "❌ hello_module.ko가 없습니다!"; \
		echo "   먼저 빌드하세요: cd ../drivers && make all"; \
	fi

remove-oled:
	sudo rmmod oled_driver 2>/dev/null || echo "OLED 드라이버가 로드되지 않았습니다"

remove-hello:
	sudo rmmod hello_module 2>/dev/null || echo "Hello 모듈이 로드되지 않았습니다"

# 빌드된 모듈로 테스트 (빌드는 하지 않음)
test:
	@if [ -f oled_driver.ko ]; then \
		echo "🧪 기존 빌드된 모듈로 테스트..."; \
		make install-oled; \
		cd ../tests/day3 && ./oled_test "Test from modules folder!"; \
		echo "📋 커널 로그:"; \
		dmesg | tail -5; \
	else \
		echo "❌ 빌드된 모듈이 없습니다!"; \
		echo "   전체 빌드 및 테스트: cd ../drivers && make test"; \
	fi

info:
	@echo "=== modules 폴더 정보 ==="
	@echo "📁 현재 위치: $(PWD)"
	@echo "🎯 용도: 빌드 결과물(.ko) 저장소"
	@echo "🔨 빌드 위치: $(PWD)/../drivers"
	@echo ""
	@echo "📦 현재 모듈들:"
	@ls -la *.ko 2>/dev/null || echo "    빌드된 모듈이 없습니다"
	@echo ""
	@echo "🔄 로드된 모듈들:"
	@lsmod | grep -E "(hello|oled)" || echo "    로드된 모듈이 없습니다"

help:
	@echo "=== modules 폴더 사용법 ==="
	@echo "🔨 빌드:           cd ../drivers && make all"
	@echo "📦 OLED 설치:      make install-oled"
	@echo "📦 Hello 설치:     make install-hello"
	@echo "🗑️  모듈 제거:      make remove-oled / remove-hello"
	@echo "🧪 테스트:         make test (빌드된 모듈로)"
	@echo "🧹 정리:          make clean"
	@echo "ℹ️  정보:           make info"

.PHONY: all clean install-oled install-hello remove-oled remove-hello test info help
