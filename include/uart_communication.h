#ifndef UART_COMMUNICATION_H
#define UART_COMMUNICATION_H

#include "smart_env_monitor.h"

// UART 설정
#define UART_DEVICE           "/dev/ttyAMA0"
#define UART_BAUDRATE         115200
#define UART_BUFFER_SIZE      256

// 통신 프로토콜 명령어
#define CMD_GET_DATA          "GET_DATA"
#define CMD_SET_TIME          "SET_TIME"
#define CMD_RESET             "RESET"
#define CMD_STATUS            "STATUS"

// 응답 코드
#define RESP_OK               "OK"
#define RESP_ERROR            "ERROR"
#define RESP_INVALID          "INVALID"

// UART 통신 구조체
typedef struct {
    int fd;
    int baudrate;
    char device_path[64];
} uart_config_t;

// 데이터 패킷 구조체
typedef struct {
    char command[32];
    char data[200];
    char checksum[8];
    struct timespec timestamp;
} uart_packet_t;

// UART 통신 함수
int uart_init(const char *device, int baudrate);
void uart_cleanup(void);
int uart_send_data(const char *data, int len);
int uart_receive_data(char *buffer, int max_len, int timeout_ms);
int uart_send_sensor_data(const sensor_data_t *data);
int uart_send_command(const char *command);
int uart_parse_command(const char *buffer, uart_packet_t *packet);

// 프로토콜 함수
int uart_process_get_data_command(void);
int uart_process_set_time_command(const char *time_str);
int uart_process_reset_command(void);
int uart_send_status_response(void);

// 유틸리티 함수
void uart_calculate_checksum(const char *data, char *checksum);
int uart_verify_checksum(const uart_packet_t *packet);
void uart_format_json_response(const sensor_data_t *data, char *json_buffer, int buffer_size);

#endif // UART_COMMUNICATION_H
