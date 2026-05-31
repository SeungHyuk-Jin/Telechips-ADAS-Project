#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>

#define SERVER_IP "192.168.2.5"
#define SERVER_PORT 5000

#define SPI_DEVICE "/dev/spidev0.0"

#define SENSOR_SEND_INTERVAL 100  // ms

int sock;

int analog_read(int fd, uint8_t channel) {
    uint8_t tx[] = {1, (8 + channel) << 4, 0};
    uint8_t rx[3] = {0,};

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 3,
        .delay_usecs = 0,
        .speed_hz = 500000,
        .bits_per_word = 8,
    };

    ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    int value = ((rx[1] & 3) << 8) + rx[2];
    return value;
}

void send_message(const char *msg) {
    if (send(sock, msg, strlen(msg), 0) < 0) {
        printf("메시지 전송 실패!\n");
    } else {
        printf("Sent: %s", msg);
    }
}

// millis() 대체용 함수
unsigned int millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

int main(void) {
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("소켓 생성 실패\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("서버 연결 실패\n");
        return 1;
    }

    printf("서버에 연결됨: %s:%d\n", SERVER_IP, SERVER_PORT);

    int fd_buttons = open("/dev/buttons_dev", O_RDONLY);
    if (fd_buttons < 0) {
        perror("버튼 디바이스 열기 실패");
        return 1;
    }

    int spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        printf("SPI 열기 실패\n");
        return 1;
    }

    uint8_t mode = 0;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &(uint8_t){8});
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &(uint32_t){500000});

    struct pollfd pfds[1];
    pfds[0].fd = fd_buttons;
    pfds[0].events = POLLIN;

    unsigned int last_sensor_time = 0;

    while (1) {
        int ret = poll(pfds, 1, 50);  // 50ms마다 체크

        if (ret > 0 && (pfds[0].revents & POLLIN)) {
            char buf[64] = {0};
            int len = read(fd_buttons, buf, sizeof(buf));
            if (len > 0) {
                printf("Button event: %s", buf);

                int btn = -1;
                sscanf(buf, "button %d", &btn);

                switch (btn) {
                    case 1: send_message("!\n"); break;
                    case 2: send_message("@\n"); break;
                    case 3: send_message("#\n"); break;
                    case 4: send_message("$\n"); break;
                    default: printf("알 수 없는 버튼 ID\n"); break;
                }
            }
        }

        unsigned int now = millis();
        if (now - last_sensor_time >= SENSOR_SEND_INTERVAL) {
            last_sensor_time = now;

            int reading1 = analog_read(spi_fd, 0);
            int reading2 = analog_read(spi_fd, 1);

            if ((reading1 > 50) || (reading2 > 50)) {
                char sensor_msg[50];
                snprintf(sensor_msg, sizeof(sensor_msg), "a:%db:%d\n", reading1,reading2);
                send_message(sensor_msg);
            }
        }
    }

    close(spi_fd);
    close(fd_buttons);
    close(sock);
    return 0;
}
