#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMD_FILE "monitor_cmd.txt"
#define ARG_FILE "monitor_args.txt"

static pid_t mon_pid = -1;
static int waiting_stop = 0;

static void on_sigchld(int sig) {
    int status;
    wait(&status);
    printf(" Monitor : a terminat statusul");
    mon_pid      = -1;
    waiting_stop = 0;
}

static void write_file(const char *path, const char *s) {
    int fis = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (fis < 0) {
        perror("open");
        exit(-1);
    }
    write(fis, s, strlen(s));
    close(fis);
}

static void trimite_comanda(const char *command, const char *args) {
    write_file(CMD_FILE, command);
    write_file(ARG_FILE, args ? args : "");
    kill(mon_pid, SIGUSR1);
}

int main() {
    struct sigaction sa = {
        .sa_handler = on_sigchld,
        .sa_flags   = SA_RESTART
    };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    char line[256];
    while (1) {
        printf("hub> "); fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';

        if (waiting_stop) {
            printf("Monitor : Asteptam sa se inchida \n");
            continue;
        }

        if (strcmp(line, "start_monitor") == 0) {
            if (mon_pid > 0) {
                printf("Monitor deja lucreaza \n");
            } else {
                mon_pid = fork();
                if (mon_pid < 0) {
                    perror("fork");
                    exit(1);
                } else if (mon_pid == 0) {
                    execl("./monitor", "./monitor", NULL);
                    perror("execl");
                    exit(1);
                } else {
                    printf("Monitor a inceput )\n");
                }
            }
        }
        else if (strcmp(line, "list_hunts") == 0) {
            if (mon_pid < 0) {
                printf("Monitor nu ruleaza\n");
            } else {
                trimite_comanda("list_hunts", "");
            }
        }
        else if (strncmp(line, "list_treasures ", 15) == 0) {
            if (mon_pid < 0) {
                printf("Monitor nu ruleaza\n");
            } else {
                trimite_comanda("list_treasures", line + 15);
            }
        }
        else if (strncmp(line, "view_treasure ", 14) == 0) {
            if (mon_pid < 0) {
                printf("Monitor nu ruleaza\n");
            } else {
                trimite_comanda("view_treasure", line + 14);
            }
        }
        else if (strcmp(line, "stop_monitor") == 0) {
            if (mon_pid < 0) {
                printf("Monitor nu ruleaza\n");
            } else {
                trimite_comanda("stop_monitor", "");
                waiting_stop = 1;
            }
        }
        else if (strcmp(line, "exit") == 0) {
            if (mon_pid > 0) {
                printf("Monitor nu ruleaza\n");
            } else {
                break;
            }
        }
        else {
            printf("Comanda ne identificata \n");
        }
    }

    return 0;
}