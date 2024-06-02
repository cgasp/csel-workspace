#define _GNU_SOURCE
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void catch_signal(int signo)  // slide 12
{
    printf("Signal %d reçu et ignoré\n", signo);
}

void capture_de_signaux()  // slide 10
{
    struct sigaction act;
    act.sa_handler = catch_signal;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}

void set_affinity(int core_id)  // sp.06 slide 63
{
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_id, &set);
    if (sched_setaffinity(0, sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int fd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {  // sp.06 slide 52
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();  // sp.06 slide 24
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {  // process enfant
        capture_de_signaux();
        set_affinity(0);  // defini CORE 0

        const char* messages[] = {"Hello", "Alive?", "exit"};
        for (int i = 0; i < 3; ++i) {
            if (write(fd[1], messages[i], strlen(messages[i]) + 1) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            sleep(1);
        }

        close(fd[1]);
        exit(EXIT_SUCCESS);
    } else {  // Parent process
        capture_de_signaux();
        set_affinity(1);  // defini CORE 1

        char buffer[128];
        while (1) {
            ssize_t count = read(fd[0], buffer, sizeof(buffer));
            if (count == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            } else if (count == 0) {
                break;
            }

            printf("Received message: %s\n", buffer);
            if (strcmp(buffer, "exit") == 0) {
                break;
            }
        }

        close(fd[0]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}
