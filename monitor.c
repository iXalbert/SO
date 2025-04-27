#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>


#include "treasure_operations.h"

#define CMD_FILE "monitor_cmd.txt"
#define ARG_FILE "monitor_args.txt"

void read_file(const char *path, char *buffer, size_t bufsz) {
    int fis = open(path, O_RDONLY);
    if (fis < 0) {
        buffer[0] = '\0';
        exit(-1);
    }

    ssize_t n = read(fis, buffer, bufsz - 1);
    close(fis);

    if (n > 0) 
    buffer[n] = '\0';
    else   
    buffer[0] = '\0';
}

void handle_sigusr1(int signo) {

    char command[128];
    char args[128];

    read_file(CMD_FILE, command, sizeof(command));

    read_file(ARG_FILE, args, sizeof(args));

    command[strcspn(command, "\n")]   = '\0';

    args[strcspn(args, "\n")] = '\0';

    if (strcmp(command, "list_hunts") == 0) {

        printf(" MOnitor:  Listarea hunt-ului:\n");

        DIR *d = opendir(".");

        struct dirent *e;
        struct stat st;

        char path[256];

        while ((e = readdir(d)) != NULL) {
            if (e->d_type == DT_DIR && strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0) {
                snprintf(path, sizeof(path), "%s/treasures.bin", e->d_name);
                if (stat(path, &st) == 0) {
                    long cnt = st.st_size / sizeof(Treasure);
                    printf("  %s: %ld treasures\n", e->d_name, cnt);
                }
            }
        }

        closedir(d);
    }
    else if (strcmp(command, "list_treasures") == 0) {

        if (args[0] != '\0') {
            lista_treasure(args);
        } else {
            printf("Monitor : nu s a gasit id-ul\n");
        }
    }
    else if (strcmp(command, "view_treasure") == 0) {

        char *hunt = strtok(args, " ");
        char *id   = strtok(NULL, " ");

        if (hunt && id) {
            view_treasure(hunt, atoi(id));
        } else {
            printf(" Monitor view_treasur gresit la arguemnte\n");
        }
    }
    else if (strcmp(command, "stop_monitor") == 0) {
        printf("Monitorul : urmeaza sa ma inchid \n");
        usleep(2000000);
        exit(0);
    }
    else {
        printf(" Comanda ne indentificata \n");
    }
}

int main() {
    struct sigaction sa = {
        .sa_handler = handle_sigusr1,
        .sa_flags   = SA_RESTART
    };
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Monitor PID=%d, asteapta comanda \n", getpid());
    while (1) {
        pause();
    }
    return 0;
}
