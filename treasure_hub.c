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
        //exit(-1)
    }

    mon_pid = fork();
    if (mon_pid < 0) {
        perror("Eroare fork");
        return;
        //exit(-1);
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

void scrie_argument(const char *arg) {
    int fd = open("monitor_args.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Eroare deschidere monitor_args.txt");
        return;
    }
    
    if (arg != NULL) {
        write(fd, arg, strlen(arg));
    }
    
    close(fd);
}

void trimite_semnal(int semnal, const char *arg) {
    if (mon_pid <= 0) {
        write(STDOUT_FILENO, "Monitor nu ruleaza\n", 20);
        return;
    }
    
    // Scrie argumentul in fisier inainte de a trimite semnal
    scrie_argument(arg);
    
    // Asteapta putin ca sa se termine scrierea
    usleep(10000);
    
    kill(mon_pid, semnal);
    citeste_de_la_monitor();
}

void calculeaza_scor(const char *hunt_name) {
    
    if (hunt_name == NULL || hunt_name[0] == '\0') {
        write(STDOUT_FILENO, "Nu exista hunt pentru a calcula scorul\n", 40);
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/treasures.bin", hunt_name);
    
    int fis_test = open(path, O_RDONLY);
    if (fis_test < 0) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg), "Fisierul %s/treasures.bin nu a putut fi deschis\n", hunt_name);
        write(STDOUT_FILENO, error_msg, strlen(error_msg));
        return;
    }
    close(fis_test);
    
    int pipe_score[2];
    if (pipe(pipe_score) < 0) {
        perror("Eroare pipe la scor \n");
        return;
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("Eroare fork la scor \n");
        close(pipe_score[0]);
        close(pipe_score[1]);
        return;
    }
    
    if (pid == 0) {

        close(pipe_score[0]); 
        
        int fis = open(path, O_RDONLY);
        if (fis < 0) {
            perror("Error opening treasures.bin file");
            exit(1);
        }
        
        dup2(fis, STDIN_FILENO);
        dup2(pipe_score[1], STDOUT_FILENO);
        
        close(fis);
        close(pipe_score[1]);
        
        execl("./score_calculator", "score_calculator", NULL);
        perror("Eroare la executarea score_calculator");
        exit(1);
    } else {

        close(pipe_score[1]); 
        
        char buff[1024];
        ssize_t n;
        
        //char header[256];
        //snprintf(header, sizeof(header), "\n--- Score Calculation for %s ---\n", hunt_name);
        //write(STDOUT_FILENO, header, strlen(header));
        
        while ((n = read(pipe_score[0], buff, sizeof(buff) - 1)) > 0) {
            buff[n] = '\0';
            write(STDOUT_FILENO, buff, n);
        }
        
        close(pipe_score[0]);
        waitpid(pid, NULL, 0);
        
        //write(STDOUT_FILENO, "--- End of Score Calculation ---\n\n", 35);
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
            trimite_semnal(SIGUSR1, NULL);
        } else if (strncmp(linie, "list_treasures ", 15) == 0) {
            const char *hunt_name = linie + 15;
            if (hunt_name[0] == '\0') {
                write(STDOUT_FILENO, "Eroare: Specificati hunt-ul (list_treasures HUNT_NAME)\n", 55);
            } else {
                trimite_semnal(SIGUSR2, hunt_name);
            }
        } else if (strncmp(linie, "view_treasure ", 14) == 0) {
            const char *args = linie + 14;
            if (args[0] == '\0') {
                write(STDOUT_FILENO, "Eroare: Specificati hunt-ul si ID-ul (view_treasure HUNT_NAME TREASURE_ID)\n", 75);
            } else {
                trimite_semnal(SIGINT, args);
            }
        } else if (strncmp(linie, "calculate_score", 15) == 0) {
            if (linie[15] == '\0') {
                write(STDOUT_FILENO, "Monitor: calculate_score <hunt_name>\n", 37);
            } else if (linie[15] == ' ') {
                calculeaza_scor(linie + 16);
            } else {
                write(STDOUT_FILENO, "Eroare la scriere. Use: calculate_score <hunt_name>\n", 51);
            }
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
