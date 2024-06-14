/*
controlé par daemon: /etc/init.d/S99daemon
*/

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

#include "ssd1306.h"

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

#define FIFO_PATH "/tmp/miniproj_fifo"
#define BUFFER_SIZE 256

static int open_LED()
{
    // from silly_led_control exercise

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
    // from silly_led_control exercise

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

float read_CPU_temp()
{
    // https://mse-csel.github.io/website/assignments/programmation-noyau/modules/#acces-aux-entreessorties
    // https://stackoverflow.com/q/65002876
    FILE* fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL) {
        perror("Failed to open temperature file");
        return -1.0;
    }

    char temp_str[6];
    if (fgets(temp_str, sizeof(temp_str), fp) == NULL) {
        perror("Failed to read temperature");
        fclose(fp);
        return -1.0;
    }

    fclose(fp);

    int temp = atoi(temp_str);
    return temp / 1000.0;
}

void initialize_display()
{
    ssd1306_init();
    ssd1306_set_position(0, 0);
    ssd1306_puts("CSEL1a - SP.07");
    ssd1306_set_position(0, 1);
    ssd1306_puts("  Demo - SW");
    ssd1306_set_position(0, 2);
    ssd1306_puts("--------------");
}

void update_display_line(int line, const char* content)
{
    // re-écrie la ligne du display
    ssd1306_set_position(0, line);
    ssd1306_puts(content);
    syslog(LOG_INFO, "OLED contenu MaJ [ligne %d] %s", line, content);
}

void update_temperature_display(float CPU_temp)
{
    char temp_str[50];
    snprintf(temp_str, sizeof(temp_str), "Temp: %.2f'C", CPU_temp);
    update_display_line(3, temp_str);
}

int main(int argc, char* argv[])
{
    long period = 500;  // ms (période pour 2 Hz)
    if (argc >= 2) period = atoi(argv[1]);
    period *= 1000000000;  // in ns

    long p = period / 2;  // 2 cycles per second (2Hz)

    int led      = open_LED();
    int button_1 = configure_button(GPIO_BUTTON_1, BUTTON_1);
    int button_2 = configure_button(GPIO_BUTTON_2, BUTTON_2);
    int button_3 = configure_button(GPIO_BUTTON_3, BUTTON_3);

    char buf[2];
    int frequency     = 2;  // frequence initial
    int new_frequency = 0;  // frequence initial

    // commence en mode auto, frequence s'adapte à la temperature
    char mode[]   = "auto  ";
    int auto_mode = 1;

    // syslog instantiation
    openlog("mini-proj daemon", LOG_PID | LOG_CONS, LOG_USER);

    // epoll instance creation
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // epoll_event initialization
    struct epoll_event event;
    struct epoll_event events[4];  // sp.05 slide 72

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

    // creation FIFO
    // elimine le FIFO pour le re-creer
    if (remove(FIFO_PATH) == 0) {
        syslog(LOG_INFO, "restarted FIFO file\n");
        // printf("removed FIFO file\n");
        // fflush(stdout);
    } else {
        syslog(LOG_INFO, "Unable to delete FIFO file\n");
        // printf("Unable to delete FIFO file\n");
        // fflush(stdout);
    }
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    // ouvre FIFO et garde FD pour epoll
    int fifo_fd = open(FIFO_PATH, O_RDWR | O_NONBLOCK);
    if (fifo_fd == -1) {
        perror("open fifo");
        exit(EXIT_FAILURE);
    }

    event.events  = EPOLLIN | EPOLLPRI | EPOLLET;  // Add more events if needed
    event.data.fd = fifo_fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fifo_fd, &event) == -1) {
        perror("epoll_ctl: fifo_fd");
        exit(EXIT_FAILURE);
    }

    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    syslog(LOG_INFO, "Daemon App Started");

    int k = 0;

    // Initialize display
    initialize_display();

    // CPU temp
    float CPU_temp      = read_CPU_temp();
    float last_CPU_temp = 0;
    if (CPU_temp != -1.0) {
        update_temperature_display(CPU_temp);
        last_CPU_temp = CPU_temp;
    }

    struct timespec t3;
    t3 = t1;

    // Initial display setup
    char freq_str[14];
    snprintf(freq_str, sizeof(freq_str), "Freq: %dHz", frequency);
    update_display_line(4, freq_str);

    update_display_line(5, "Duty: 50%");

    char mode_str[14];
    snprintf(mode_str, sizeof(mode_str), "Mode: %s", mode);
    update_display_line(6, mode_str);

    syslog(LOG_INFO, "LCD displayed");

    printf("Daemon App Started\n");
    fflush(stdout);

    // status initial
    printf("Initial Frequency: %dHz, Mode: %s, Temperature: %.2f'C\n",
           frequency,
           mode,
           CPU_temp);
    fflush(stdout);

    while (1) {
        struct timespec t2;
        clock_gettime(CLOCK_MONOTONIC, &t2);

        // sp.07 slide 15
        long delta =
            (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);

        int toggle =
            delta >=
            p;  // ((k == 0) && (delta >= p1)) || ((k == 1) && (delta >= p2));
        if (toggle) {
            t1 = t2;
            k  = (k + 1) % 2;
            if (k == 0) {
                pwrite(led, "1", sizeof("1"), 0);
            } else {
                pwrite(led, "0", sizeof("0"), 0);
            }
        }

        struct timespec t4;
        clock_gettime(CLOCK_MONOTONIC, &t4);

        long delta_t3t4 =
            (t4.tv_sec - t3.tv_sec) * 1000000000 + (t4.tv_nsec - t3.tv_nsec);

        // Verifie la temperature chaque seconde
        if (delta_t3t4 >= 1000000000) {
            t3 = t4;
            // printf("temp checked");
            // fflush(stdout);

            float CPU_temp = read_CPU_temp();
            // si CPU temp valide et different de la derniÈre mesure
            if (CPU_temp != -1.0 && CPU_temp != last_CPU_temp) {
                // printf("display temp update: %.2f\n", CPU_temp); // debug
                // fflush(stdout);
                update_temperature_display(CPU_temp);
                last_CPU_temp = CPU_temp;
                // si mode auto, met à jour la frequence automatiquement
                if (auto_mode == 1) {
                    if (CPU_temp != -1.0) {
                        if (CPU_temp < 30.0) {
                            new_frequency = 2;
                        } else if (CPU_temp < 35.0) {
                            new_frequency = 2;
                        } else if (CPU_temp < 40.0) {
                            new_frequency = 5;
                        } else if (CPU_temp < 45.0) {
                            new_frequency = 10;
                        } else {
                            new_frequency = 20;
                        }

                        if (new_frequency != frequency) {
                            frequency = new_frequency;
                            syslog(LOG_INFO,
                                   "Auto mode: Frequency set to %d Hz "
                                   "based on CPU temp %.1f°C",
                                   frequency,
                                   CPU_temp);
                            snprintf(freq_str,
                                     sizeof(freq_str),
                                     "Freq: %dHz",
                                     frequency);
                            update_display_line(4, freq_str);
                        }
                    }
                }
            }
        }

        // Wait for an event (button press or FIFO command)
        int nr = epoll_wait(epfd, events, 4, 100);  // timeout 100ms
        if (nr == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        // Event processing
        for (int i = 0; i < nr; i++) {
            int fd = events[i].data.fd;

            // Prend en charge commande FIFO
            if (fd == fifo_fd) {
                char command[BUFFER_SIZE];
                ssize_t bytes_read = read(fifo_fd, command, BUFFER_SIZE);
                if (bytes_read > 0) {
                    command[bytes_read] = '\0';
                    if (strncmp(command, "freq ", 5) == 0) {
                        int new_frequency = atoi(command + 5);
                        if (new_frequency > 0) {
                            frequency = new_frequency;
                            syslog(LOG_INFO,
                                   "Frequency set to %d Hz via FIFO",
                                   frequency);
                            snprintf(freq_str,
                                     sizeof(freq_str),
                                     "Freq: %dHz",
                                     frequency);
                            update_display_line(4, freq_str);

                            // desactive AUTO mode
                            auto_mode = 0;
                            syslog(LOG_INFO,
                                   "Mode set to MANUAL due to Frequency set by "
                                   "FIFO");
                            snprintf(mode_str,
                                     sizeof(mode_str),
                                     "Mode: %s",
                                     "manual");
                            update_display_line(6, mode_str);
                        }
                    } else if (strncmp(command, "mode ", 5) == 0) {
                        if (strncmp(command + 5, "auto", 4) == 0) {
                            auto_mode = 1;
                            syslog(LOG_INFO, "Mode set to AUTO via FIFO");
                            snprintf(mode_str,
                                     sizeof(mode_str),
                                     "Mode: %s",
                                     "auto  ");
                            update_display_line(6, mode_str);
                        } else if (strncmp(command + 5, "manual", 6) == 0) {
                            auto_mode = 0;
                            syslog(LOG_INFO, "Mode set to MANUAL via FIFO");
                            snprintf(mode_str,
                                     sizeof(mode_str),
                                     "Mode: %s",
                                     "manual");
                            update_display_line(6, mode_str);
                        } else {
                            printf(
                                "Invalid mode command. Use 'mode auto' or "
                                "'mode manual'\n");
                        }
                    }
                }
            } else {  // Prend en charge pression de bouton
                lseek(fd, 0, SEEK_SET);
                read(fd, buf, sizeof(buf));
                if (buf[0] == '1') {
                    if (fd == button_1) {
                        frequency++;
                        syslog(LOG_INFO,
                               "Frequency increased to %d Hz",
                               frequency);
                        snprintf(freq_str,
                                 sizeof(freq_str),
                                 "Freq: %dHz",
                                 frequency);
                        update_display_line(4, freq_str);
                        auto_mode = 0;
                    } else if (fd == button_2) {
                        frequency--;
                        if (frequency < 1)
                            frequency = 1;  // Ensure frequency doesn't go
                                            // below 1 Hz
                        syslog(LOG_INFO,
                               "Frequency decreased to %d Hz",
                               frequency);
                        snprintf(freq_str,
                                 sizeof(freq_str),
                                 "Freq: %dHz",
                                 frequency);
                        update_display_line(4, freq_str);
                        auto_mode = 0;
                    } else if (fd == button_3) {
                        if (auto_mode) {
                            auto_mode = 0;
                        } else {
                            auto_mode = 1;
                        }

                        if (auto_mode) {
                            snprintf(
                                mode_str, sizeof(mode_str), "Mode: auto  ");
                            syslog(LOG_INFO,
                                   "Mode toggled to AUTO via button press");
                        } else {
                            snprintf(
                                mode_str, sizeof(mode_str), "Mode: manual");
                            syslog(LOG_INFO,
                                   "Mode toggled to MANUAL via button press");
                        }

                        update_display_line(6, mode_str);
                    }

                    if (auto_mode == 0 && (fd == button_1 || fd == button_2)) {
                        // boutton presser desactive AUTO mode
                        syslog(LOG_INFO,
                               "Mode set to MANUAL due to button pressed");
                        snprintf(
                            mode_str, sizeof(mode_str), "Mode: %s", "manual");
                        update_display_line(6, mode_str);
                    }
                }
            }
            // met à jour les periodes de clignotement
            period = (1000 / frequency) * 1000000;
            p      = period / 2;
        }
    }

    // Close syslog
    closelog();

    close(led);
    close(button_1);
    close(button_2);
    close(button_3);
    close(fifo_fd);
    close(epfd);

    return 0;
}
