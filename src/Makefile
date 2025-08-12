# 크로스 컴파일 설정
CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L
INCLUDES = -I../include
LIBS = -lgpiod

# 타겟 및 소스 파일
TARGET = smart_env_monitor
SOURCES = $(wildcard *.c */*.c)
OBJECTS = $(SOURCES:.c=.o)

# 기본 빌드
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "크로스 컴파일러로 링킹: $(CC)"
	$(CC) $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	@echo "컴파일 중: $< ($(CC))"
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	find . -name "*.o" -delete

.PHONY: all clean
