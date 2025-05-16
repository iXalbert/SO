#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>


#include "treasure_operations.h"

// Variabila globala pentru stocarea argumentelor comenzilor
char hunt_arg[256] = {0};

// Prototipuri de functii
void read_hunt_argument();
// SIGUSR1 handler - list_hunts command
void handle_sigusr1(int signo) {
    DIR *d = opendir(".");
    struct dirent *entry;
    char output_buffer[512];
    
    if (!d) {
        write(STDOUT_FILENO, "Eroare: Nu s-a putut deschide directorul curent\n", 48);
        return;
    }
    
    while((entry = readdir(d)) != NULL) {
        if(entry->d_type == DT_DIR && strncmp(entry->d_name, "Hunt", 4) == 0) {
            write(STDOUT_FILENO, entry->d_name, strlen(entry->d_name));
            write(STDOUT_FILENO, "\n", 1);
        }
    }
    
    closedir(d);
}

// SIGUSR2 handler - list_treasures command
void handle_sigusr2(int signo){
    // Citeste argumentul la inceputul handler-ului
    read_hunt_argument();
    
    // Metoda mai sigura: folosim un singur buffer mare pentru tot output-ul
    char huge_buffer[4096] = {0};
    char *p = huge_buffer;
    int remaining = sizeof(huge_buffer) - 1;
    
    DIR *d = opendir(".");
    if (!d) {
        write(STDOUT_FILENO, "Eroare: Nu pot deschide directorul curent\n", 42);
        return;
    }
    
    struct dirent *entry;
    int found = 0;
    
    if (hunt_arg[0] == '\0') {
        write(STDOUT_FILENO, "Eroare: Nu s-a specificat numele hunt-ului\n", 42);
        closedir(d);
        return;
    }
    
    // Adauga o linie goala la inceput
    p += snprintf(p, remaining, "\n");
    remaining = sizeof(huge_buffer) - (p - huge_buffer);
    
    while((entry = readdir(d)) != NULL){
        if(entry->d_type == DT_DIR && strcmp(entry->d_name, hunt_arg) == 0){
            found = 1;
            
            char drum[256];
            snprintf(drum, sizeof(drum), "%s/treasures.bin", entry->d_name);
            
            int fis = open(drum, O_RDONLY);
            if (fis < 0) {
                p += snprintf(p, remaining, "Nu s-a putut deschide fisierul %s/treasures.bin\n", entry->d_name);
                remaining = sizeof(huge_buffer) - (p - huge_buffer);
                continue;
            }
            
            // Verificam daca fisierul este gol sau invalid
            struct stat st;
            if (fstat(fis, &st) == 0 && st.st_size == 0) {
                p += snprintf(p, remaining, "Fisierul de comori este gol\n");
                remaining = sizeof(huge_buffer) - (p - huge_buffer);
                close(fis);
                continue;
            }
            
            // Adauga headerul tabelului
            p += snprintf(p, remaining, "Lista comori pentru %s:\n", entry->d_name);
            remaining = sizeof(huge_buffer) - (p - huge_buffer);
            
            p += snprintf(p, remaining, "+------+------------------+----------+\n");
            remaining = sizeof(huge_buffer) - (p - huge_buffer);
            
            p += snprintf(p, remaining, "| ID   | Username         | Valoare  |\n");
            remaining = sizeof(huge_buffer) - (p - huge_buffer);
            
            p += snprintf(p, remaining, "+------+------------------+----------+\n");
            remaining = sizeof(huge_buffer) - (p - huge_buffer);
            
            // Citeste si afiseaza toate comorile
            Treasure treasure;
            int count = 0;
            int total_valoare = 0;
            
            while(read(fis, &treasure, sizeof(Treasure)) == sizeof(Treasure)){
                count++;
                total_valoare += treasure.value;
                
                // Limiteaza username la 16 caractere pentru afisare uniforma
                char username_display[17] = {0};
                strncpy(username_display, treasure.username, 16);
                
                p += snprintf(p, remaining, "| %-4d | %-16s | %-8d |\n",
                        treasure.treasure_id,
                        username_display,
                        treasure.value);
                
                remaining = sizeof(huge_buffer) - (p - huge_buffer);
                
                // Verificare overflow
                if (remaining < 100) {
                    // Buffer prea plin, trimite ce avem si reseteaza
                    write(STDOUT_FILENO, huge_buffer, strlen(huge_buffer));
                    p = huge_buffer;
                    *p = '\0';
                    remaining = sizeof(huge_buffer) - 1;
                }
            }
            
            // Footer tabel
            p += snprintf(p, remaining, "+------+------------------+----------+\n");
            remaining = sizeof(huge_buffer) - (p - huge_buffer);
            
            // Sumar
            if (count == 0) {
                p += snprintf(p, remaining, "Nu au fost gasite comori in acest hunt\n");
            } else {
                p += snprintf(p, remaining, "Total comori: %d | Valoare totala: %d\n", 
                        count, total_valoare);
            }
            
            close(fis);
        }
    }
    
    if (!found) {
        p += snprintf(p, remaining, "Eroare: Hunt '%s' nu a fost gasit\n", hunt_arg);
    }
    
    closedir(d);
    
    // Trimite tot buffer-ul la o singura scriere
    write(STDOUT_FILENO, huge_buffer, strlen(huge_buffer));
    
    // Goleste argumentul dupa utilizare
    hunt_arg[0] = '\0';
}

// SIGINT handler - view_treasure command
void handle_sigint(int signo) {
    // Citeste argumentul la inceputul handler-ului
    read_hunt_argument();
    
    DIR *d = opendir(".");
    struct dirent *entry;
    int found = 0;
    char output_buffer[512];
    int treasure_id = 0;
    char hunt_name[128];
    
    // Parse hunt name and treasure ID
    char *id_str = strchr(hunt_arg, ' ');
    if (id_str == NULL) {
        sprintf(output_buffer, "Eroare: Format corect este 'HUNT_NAME TREASURE_ID'\n");
        write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
        closedir(d);
        hunt_arg[0] = '\0';
        return;
    }
    
    // Split string
    int hunt_len = id_str - hunt_arg;
    strncpy(hunt_name, hunt_arg, hunt_len);
    hunt_name[hunt_len] = '\0';
    
    // Get ID
    id_str++;
    treasure_id = atoi(id_str);
    
    if (treasure_id <= 0) {
        sprintf(output_buffer, "Eroare: ID de comoara invalid: %s\n", id_str);
        write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
        closedir(d);
        hunt_arg[0] = '\0';
        return;
    }
    
    while((entry = readdir(d)) != NULL){
        if(entry->d_type == DT_DIR && strcmp(entry->d_name, hunt_name) == 0){
            found = 1;
            
            char drum[256];
            snprintf(drum, sizeof(drum), "%s/treasures.bin", entry->d_name);
            
            int fis = open(drum, O_RDONLY);
            
            if(fis >= 0){
                Treasure treasure;
                int treasure_found = 0;
                
                while(read(fis, &treasure, sizeof(Treasure)) == sizeof(Treasure)){
                    if (treasure.treasure_id == treasure_id) {
                        treasure_found = 1;
                        
                        // Detailed view of single treasure
                        sprintf(output_buffer, 
                                "\nDetalii Comoara (ID: %d)\n"
                                "---------------------------\n"
                                "Username: %s\n"
                                "Coordonate: %.2f, %.2f\n"
                                "Indiciu: %s\n"
                                "Valoare: %d\n"
                                "---------------------------\n",
                                treasure.treasure_id,
                                treasure.username,
                                treasure.latitude, treasure.longitude,
                                treasure.clue,
                                treasure.value);
                        
                        write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                        break;
                    }
                }
                
                if (!treasure_found) {
                    sprintf(output_buffer, "Eroare: Comoara cu ID-ul %d nu a fost gasita in %s\n", 
                            treasure_id, entry->d_name);
                    write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
                }
                
                close(fis);
            } else {
                sprintf(output_buffer, "Eroare: Nu s-a putut deschide fisierul %s/treasures.bin\n", entry->d_name);
                write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
            }
        }
    }
    
    if (!found) {
        sprintf(output_buffer, "Eroare: Hunt-ul '%s' nu a fost gasit\n", hunt_name);
        write(STDOUT_FILENO, output_buffer, strlen(output_buffer));
    }
    
    closedir(d);
    
    hunt_arg[0] = '\0';
}

void read_hunt_argument() {
    int fis = open("monitor_args.txt", O_RDONLY);
    if (fis < 0) {
        hunt_arg[0] = '\0';
        return;
    }
    
    // Goleste buffer-ul inainte de citire
    memset(hunt_arg, 0, sizeof(hunt_arg));
    
    ssize_t n = read(fis, hunt_arg, sizeof(hunt_arg) - 1);
    close(fis);
    
    if (n <= 0) {
        hunt_arg[0] = '\0';
        return;
    }
   
    hunt_arg[n] = '\0';
    
    // Elimina whitespace de la sfarsit
    size_t len = strlen(hunt_arg);
    while (len > 0 && (hunt_arg[len-1] == '\n' || hunt_arg[len-1] == '\r' || hunt_arg[len-1] == ' ')) {
        hunt_arg[len-1] = '\0';
        len--;
    }
}

int main() {
    struct sigaction sa1 = {0}, sa2 = {0}, sa3 = {0};
    
    sa1.sa_flags = SA_RESTART;
    sa2.sa_flags = SA_RESTART;
    sa3.sa_flags = SA_RESTART;
    
    sa1.sa_handler = handle_sigusr1;
    sa2.sa_handler = handle_sigusr2;
    sa3.sa_handler = handle_sigint;
    
    sigemptyset(&sa1.sa_mask);
    sigemptyset(&sa2.sa_mask);
    sigemptyset(&sa3.sa_mask);
    
    if (sigaction(SIGUSR1, &sa1, NULL) < 0) {
        perror("Fatal la SIGUSR1 handler");
        return 1;
    }
    
    if (sigaction(SIGUSR2, &sa2, NULL) < 0) {
        perror("Fatal la SIGUSR2 handler");
        return 1;
    }
    
    if (sigaction(SIGINT, &sa3, NULL) < 0) {
        perror("Fatal la SIGINT handler");
        return 1;
    }
    
    // Nu mai afisam mesajul de startup
    
    while (1) {
        // Doar asteapta semnale - citirea argumentelor se face in handlere
        pause();
    }
    
    return 0;
}
