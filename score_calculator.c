#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(){

    char buff[512];
    int scor_total = 0;

    while(fgets(buff, sizeof(buff), stdin)){

        int value = 0;

        if(strstr(buff, "Value : ")){
            sscanf(buff, "VAlue: %d", &value);
            scor_total = scor_total + value;
        }
    }

    printf("Total Score : %d \n", scor_total);
//
    return 0;
}