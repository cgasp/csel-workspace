#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_LED "/sys/class/gpio/gpio10"
#define LED "10"
#define GPIO_BUTTON_1 "/sys/class/gpio/gpio0"
#define GPIO_BUTTON_2 "/sys/class/gpio/gpio2"
#define GPIO_BUTTON_3 "/sys/class/gpio/gpio3"
#define BUTTON_1 "0"
#define BUTTON_2 "2"
#define BUTTON_3 "3"

static int open_led()
{
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, LED, strlen(LED));
    close(f);

    // config pin
    f = open(GPIO_LED "/direction", O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    f = open(GPIO_LED "/value", O_RDWR);
    return f;
}

static int configure_button(const char* gpio, const char* number)
{
    char path[256];

    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, number, strlen(number));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, number, strlen(number));
    close(f);

    // config pin
    sprintf(path, "%s/direction", gpio);
    f = open(path, O_WRONLY);
    write(f, "in", 2);
    close(f);

    // falling edge
    sprintf(path, "%s/edge", gpio);
    f = open(path, O_WRONLY);
    write(f, "both", 4);
    close(f);

    // open gpio value attribute
    sprintf(path, "%s/value", gpio);
    f = open(path, O_RDONLY);
    if (f < 0) {
        perror("Failed to open GPIO value");
    }
    return f;
}

int main(int argc, char* argv[])
{
    long period = 500;  // ms (période pour 2 Hz)
    if (argc >= 2) period = atoi(argv[1]);
    period *= 1000000;  // in ns

    long p1 = period / 2;  // 2 cycles per second (2Hz)
    long p2 = period / 2;

    int led      = open_led();
    int button_1 = configure_button(GPIO_BUTTON_1, BUTTON_1);
    int button_2 = configure_button(GPIO_BUTTON_2, BUTTON_2);
    int button_3 = configure_button(GPIO_BUTTON_3, BUTTON_3);

    char buf[2];
    int frequency = 2;  // frequence initiale

    // syslog instantiation
    openlog("silly_led_control", LOG_PID | LOG_CONS, LOG_USER);

    // epoll instance creation
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // epoll_event initialization
    struct epoll_event event;
    struct epoll_event events[3]; //sp.05 slide 72

    event.events  = EPOLLPRI;
    event.data.fd = button_1;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, button_1, &event) == -1) {
        perror("epoll_ctl: button_1");
        exit(EXIT_FAILURE);
    }

    event.data.fd = button_2;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, button_2, &event) == -1) {
        perror("epoll_ctl: button_2");
        exit(EXIT_FAILURE);
    }

    event.data.fd = button_3;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, button_3, &event) == -1) {
        perror("epoll_ctl: button_3");
        exit(EXIT_FAILURE);
    }

    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    syslog(LOG_INFO, "app start");

    printf("app start\n");  // debug print
    fflush(stdout);         // Flush the output buffer

    int k = 0;
    while (1) {
        struct timespec t2;
        clock_gettime(CLOCK_MONOTONIC, &t2);

        long delta =
            (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);

        int toggle = ((k == 0) && (delta >= p1)) | ((k == 1) && (delta >= p2));
        if (toggle) {
            t1 = t2;
            k  = (k + 1) % 2;
            if (k == 0) {
                pwrite(led, "1", sizeof("1"), 0);
                printf("LED ON\n");  // Debug print
                fflush(stdout);
            } else {
                pwrite(led, "0", sizeof("0"), 0);
                printf("LED OFF\n");  // Debug print
                fflush(stdout);
            }
        }

        // attente d'un bouton pressé 
        int nr = epoll_wait(epfd, events, 3,
                            100);  // timeout 100ms
        if (nr == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        // printf("nr %d", nr);
        // fflush(stdout);

        for (int i = 0; i < nr; i++) {
            printf("event=%d on fd=%d\n", events[i].events, events[i].data.fd);

            int fd = events[i].data.fd;
            lseek(fd, 0, SEEK_SET);
            read(fd, buf, sizeof(buf));
            if (buf[0] == '1') {
                if (events[i].data.fd == button_1) {
                    frequency++;
                    syslog(LOG_INFO, "Frequency increased to %d Hz", frequency);
                    printf("Button 1 pressed. Frequency increased to %d Hz\n",
                           frequency);  // debug print
                    fflush(stdout);     // Flush the output buffer
                } else if (events[i].data.fd == button_2) {
                    frequency = 2;
                    syslog(LOG_INFO, "Frequency reset to %d Hz", frequency);
                    printf("Button 2 pressed. Frequency reset to %d Hz\n",
                           frequency);  // debug print
                    fflush(stdout);     // Flush the output buffer
                } else if (events[i].data.fd == button_3) {
                    frequency--;
                    if (frequency < 1)
                        frequency =
                            1;  // Ensure frequency doesn't go below 1 Hz
                    syslog(LOG_INFO, "Frequency decreased to %d Hz", frequency);
                    printf("Button 3 pressed. Frequency decreased to %d Hz\n",
                           frequency);  // debug print
                    fflush(stdout);     // Flush the output buffer
                }

                // Update period and duty cycle
                period = (1000 / frequency) * 1000000;
                p1     = period / 2;
                p2     = period / 2;

                // Convert nanoseconds to seconds
                double period_sec = period / 1000000000.0;
                double p1_sec     = p1 / 1000000000.0;
                double p2_sec     = p2 / 1000000000.0;

                printf("Period: %.2f, p1: %.2f, p2: %.2f\n",
                       period_sec,
                       p1_sec,
                       p2_sec);
                fflush(stdout);
            }
        }
    }

    // Close syslog
    closelog();

    close(led);
    close(button_1);
    close(button_2);
    close(button_3);
    close(epfd);

    return 0;
}
