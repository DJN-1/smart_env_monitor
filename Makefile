# 크로스 컴파일 환경 설정
CROSS_COMPILE ?= aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc
ARCH = arm64

# 라즈베리파이 타겟 설정
RPI_USER = woo
RPI_HOST = 라즈베리파이IP주소
RPI_PATH = /home/$(RPI_USER)/smart_env_monitor

# 커널 소스 경로 (크로스 컴파일용)
KERNEL_DIR ?= $(HOME)/linux-rpi

# 빌드 타겟
all: userspace

userspace:
	@echo "=== 크로스 컴파일 빌드 시작 ==="
	@echo "컴파일러: $(CC)"
	$(MAKE) -C src/ CC=$(CC) CROSS_COMPILE=$(CROSS_COMPILE)

modules:
	@echo "=== 커널 모듈 크로스 컴파일 ==="
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/drivers ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	@echo "=== 빌드 파일 정리 ==="
	$(MAKE) -C src/ clean
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD)/drivers clean 2>/dev/null || true

deploy:
	@echo "=== 라즈베리파이로 배포 ==="
	rsync -av --exclude='.git' ./ $(RPI_USER)@$(RPI_HOST):$(RPI_PATH)/

info:
	@echo "=== 크로스 컴파일 환경 정보 ==="
	@echo "CROSS_COMPILE: $(CROSS_COMPILE)"
	@echo "CC: $(CC)"
	@echo "ARCH: $(ARCH)"
	@echo "KERNEL_DIR: $(KERNEL_DIR)"
	@echo "RPI_TARGET: $(RPI_USER)@$(RPI_HOST):$(RPI_PATH)"

.PHONY: all userspace modules clean deploy info
