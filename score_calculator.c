#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "treasure_operations.h"

int main() {
    Treasure treasure;
    int total_score = 0;
    int items_counted = 0;
    int has_error = 0;

    while (read(STDIN_FILENO, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {

        total_score += treasure.value;
        items_counted++;
    }

    if (items_counted == 0) {

        char buffer[512];
        lseek(STDIN_FILENO, 0, SEEK_SET);  
        
        while (read(STDIN_FILENO, buffer, sizeof(buffer)) > 0) {
            has_error = 1;
            break;
        }
    }

    if (has_error) {
        fprintf(stderr, "Nu a s a putut deschide fisierul binar\n");
    }
    
    printf("Iteme total: %-18d \n", items_counted);
    printf("Scor total:     %-18d \n", total_score);

    return 0;
}
