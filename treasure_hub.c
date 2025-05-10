#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

pid_t mon_pid = -1;
int pipe_fd[2]; 

void start_monitor() {
    if (mon_pid > 0) {
        write(STDOUT_FILENO, "Monitor deja ruleaza\n", 23);
        return;
    }

    if (pipe(pipe_fd) < 0) {
        perror("Eroare pipe");
        return;
    }

    mon_pid = fork();
    if (mon_pid < 0) {
        perror("Eroare fork");
        return;
    }

    if (mon_pid == 0) {
        close(pipe_fd[0]); 
        dup2(pipe_fd[1], STDOUT_FILENO); 
        execl("./monitor", "monitor", NULL);
        perror("Eroare execl");
        exit(1);
    }

    close(pipe_fd[1]); 
    write(STDOUT_FILENO, "Monitor pornit\n", 17);
}

void stop_monitor() {
    if (mon_pid <= 0) {
        write(STDOUT_FILENO, "Monitor nu ruleaza\n", 21);
        return;
    }

    kill(mon_pid, SIGTERM);
    waitpid(mon_pid, NULL, 0);
    close(pipe_fd[0]);
    mon_pid = -1;
    write(STDOUT_FILENO, "Monitor oprit\n", 14);
}

void citeste_de_la_monitor() {
    char buf[1024];
    ssize_t n = read(pipe_fd[0], buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        write(STDOUT_FILENO, buf, n);
    } else {
        write(STDOUT_FILENO, "Nu am primit date de la monitor\n", 33);
    }
}

void trimite_semnal(int semnal) {
    if (mon_pid <= 0) {
        write(STDOUT_FILENO, "Monitor nu ruleaza\n", 20);
        return;
    }

    kill(mon_pid, semnal);
    citeste_de_la_monitor();
}

void calculeaza_scor(const char *hunt_name) {
    pid_t pid = fork();
    if (pid == 0) {
        char path[256];
        snprintf(path, sizeof(path), "%s/treasures.dat", hunt_name);
        int fd = open(path, O_RDONLY);
        if (fd < 0) {
            perror("Eroare deschidere treasures.dat");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        execl("./score_calculator", "score_calculator", NULL);
        perror("Eroare execl score_calculator");
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
    }
}

int main() {
    char linie[256];

    while (1) {
        write(STDOUT_FILENO, "hub> ", 5);
        ssize_t n = read(STDIN_FILENO, linie, sizeof(linie) - 1);
        if (n <= 0) break;

        linie[n] = '\0';
        linie[strcspn(linie, "\n")] = '\0';

        if (strcmp(linie, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(linie, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(linie, "list_hunts") == 0) {
            trimite_semnal(SIGUSR1);
        } else if (strncmp(linie, "list_treasures ", 15) == 0) {
            trimite_semnal(SIGUSR2);
        } else if (strncmp(linie, "view_treasure ", 14) == 0) {
            trimite_semnal(SIGINT);
        } else if (strncmp(linie, "calculate_score ", 16) == 0) {
            calculeaza_scor(linie + 16);
        } else if (strcmp(linie, "exit") == 0) {
            if (mon_pid > 0) {
                write(STDOUT_FILENO, "Inca merge monitorul\n", 30);
            } else {
                break;
            }
        } else {
            write(STDOUT_FILENO, "Comanda necunoscuta\n", 22);
        }
    }

    return 0;
}
