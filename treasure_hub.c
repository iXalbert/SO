#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMD_FILE "monitor_comenzi.txt"
#define ARG_FILE "monitor_args.txt"

int monitor_running = 0;
int monitor_exit = 0;
pid_t monitor_pid = -1;

void sigchld_handler(int signo){

    int status ;
    pid_t pid = waitpid(monitor_pid, &status, WNOHANG);

    if(pid > 0){
        monitor_running = 0;
        monitor_exit = 0;
        printf("MONITOR - Status terminat \n");
    }
}

void start_monitor(){

    if(monitor_running){
        printf("Monitor - deja ruleaza \n");
        exit(-1);
    }

    monitor_pid = fork();

    if(monitor_pid < 0){
        perror("Eroare la fork \n");
        exit(-1);
    }
    else if(monitor_pid == 0){
        execl("./monitor", "./monitor", NULL);
        perror("execl eroare");
        exit(-1);
    }
    else{
        monitor_running = 1;
        printf("Monitor - a inveput cu PID %d \n", monitor_pid);
    }

}

void send_comanda(const char* comanda, const char *args){

    if(!monitor_running){
        perror("Monitorul - nu ruleaza \n");
        exit(-1);
    }

    int file_comand = open(CMD_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(file_comand < 0){
        perror("Eroare la deschiderea fisierului file_comand");
        exit(-1);
    }

    write(file_comand,comanda,strlen(comanda));
    close(file_comand);

    int file_args = open(ARG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(file_args < 0){
        perror("Eroare la deschiderea fisierului file_args");
        exit(-1);
    }

    write(file_args,args,strlen(args));
    close(file_args);

    kill(monitor_pid, SIGUSR1);

}

void monitor_stop(){

    if(!monitor_running){
        printf("Monitor nu ruleaza deci nu am ce inchide \n");
        exit(-1);
    }

    send_comanda("stop_monitor", "");
    monitor_exit = 1;
    printf("Comanda de stop trimisa \n");
    printf("Asteptare..... \n");
}

int main(void){

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_handler);
    sa.sa_flags = SA_RESTART;

    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("eroare la sigaction \n");
        exit(-1);
    }

    char line[256];

    while(1){
        printf(">> ");
        fflush(stdout);

        if(!fgets(line,sizeof(line),stdin)){
            break;
        }

        line[strcspn(line, "\n")] = 0;

        if(monitor_exit){
            printf("Asteptam sa termine ce a inceput deja.... \n");
            continue;
        }

        if(strcmp(line, "start_monitor") == 0){
            start_monitor();
        }
        else if(strcmp(line, "list_hunt") == 0){
            send_comanda("list_hunt", "");
        }
        else if(strncmp(line, "list_treasure", 14) == 0){
            char *args = line + 15; 
            send_comanda("list_treasure", args);
        }
        else if(strncmp(line, "view_treasure", 13) == 0){
            char *args = line + 14;
            send_comanda("view_treasure", args);
        }
        else if(strcmp(line, "stop_monitor") == 0){
            monitor_stop();
        }
        else if(strcmp(line, "exit_monitor") == 0){
            if(monitor_running){
                printf("eroare cuz monitor inca ruleaza pe fundal si recomandarea e ca prima oara sa primeasca stop \n");
            }
            else{
                break;
            }
        }
        else{
            printf("comanda ne identificata : %s \n", line);
        }

    }

    return 0;
}