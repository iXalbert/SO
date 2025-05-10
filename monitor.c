#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>


#include "treasure_operations.h"
/*
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
*/
void handle_sigusr1(int signo) {
    //char command[128];
    //char args[128];

    //read_file(CMD_FILE, command, sizeof(command));

    //read_file(ARG_FILE, args, sizeof(args));

    //command[strcspn(command, "\n")]   = '\0';

    //args[strcspn(args, "\n")] = '\0';

    /*if(command[0] == '\0'){
        printf("Monitor: Comanda invalida sau fisier lipsa sau alta problema \n");
        return;
    }*/

    /*if (strcmp(command, "list_hunts") == 0) {

        printf(" MOnitor:  Listarea hunt-ului:\n");

        DIR *d = opendir(".");

        if(!d){
            perror("opendir");
            return;
        }

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
        printf("Monitor : Inchiidere completa \n");
        exit(0);
    }
    else {
        printf(" Comanda ne indentificata \n");
    }*/


    //phase 3 : 

    DIR *d = opendir(".");
    struct dirent *entry;
    
    while((entry = readdir(d)) != NULL){

        if(entry->d_type == DT_DIR && strncmp(entry->d_name, "Hunt" , 4) == 0){

            write(STDOUT_FILENO, entry->d_name, strlen(entry->d_name));
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    closedir(d);
}

void handle_sigusr2(int signo){
    DIR *d = opendir(".");
    struct dirent *entry;
    
    while((entry = readdir(d)) != NULL){
        // Only process Hunt001 directory
        if(entry->d_type == DT_DIR && strcmp(entry->d_name, "Hunt001") == 0){
            
            char drum[128];
            snprintf(drum, sizeof(drum), "%s/treasures.bin", entry->d_name);
            
            int fis = open(drum, O_RDONLY);
            
            if(fis >= 0){
                Treasure treasure;
                char output_buffer[512];
                int count = 0;
                
                // Print header for the listing
                sprintf(output_buffer, "\nTreasures in %s:\n", entry->d_name);
                write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                sprintf(output_buffer, "--------------------------------------------------\n");
                write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                
                // Read each Treasure structure
                while(read(fis, &treasure, sizeof(Treasure)) == sizeof(Treasure)){
                    count++;
                    
                    // Format the treasure information into output_buffer
                    sprintf(output_buffer, 
                            "ID: %d\n"
                            "Username: %s\n"
                            "Coordinates: %.2f, %.2f\n"
                            "Clue: %s\n"
                            "Value: %d\n"
                            "--------------------------------------------------\n",
                            treasure.treasure_id,
                            treasure.username,
                            treasure.latitude, treasure.longitude,
                            treasure.clue,
                            treasure.value);
                    
                    // Write the formatted output
                    write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                }
                
                if (count == 0) {
                    sprintf(output_buffer, "Comoara nu a fost gasita %s\n", entry->d_name);
                    write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                } else {
                    sprintf(output_buffer, "Total de comori %d\n", count);
                    write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                }
                
                close(fis);
            } else {
                char output_buffer[128];
                sprintf(output_buffer, "Nu s a pututu deschide fisierul %s\n", entry->d_name);
                write(STDOUT_FILENO, output_buffer, strlen(output_buffer));\
                //exit(-1);
            }
        }
    }
    
    closedir(d);
}

int main() {
    /*
    struct sigaction sa = {
        .sa_handler = handle_sigusr1,
        .sa_flags   = SA_RESTART
    };
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }*/

    struct sigaction sa1 = {0}, sa2 = {0};
    
    sa1.sa_handler = handle_sigusr1;
    sa2.sa_handler = handle_sigusr2;

    sigaction(SIGUSR1, &sa1, NULL);
    sigaction(SIGUSR2, &sa2, NULL);

    //printf("Monitor PID=%d, asteapta comanda \n", getpid());
    while (1) {
        pause();
    }
    return 0;
}
