#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define USERNAME_SIZE 32
#define CLUE_SIZE 128
#define FILENAME_SIZE 64
#define LOG_SIZE 256
#define FILE_INPUT "hunt_logging"

typedef struct {
  int treasure_id;
  char username[USERNAME_SIZE];
  float latitude;
  float longitude;
  char clue[CLUE_SIZE];
  int value;
} Treasure;

void log_opperation(const char *hunt_id,const char *message){

    char path[FILENAME_SIZE];
    snprintf(path,sizeof(path),"%s,%s",hunt_id,FILE_INPUT);

    int file = open(path, O_WRONLY | O_CREAT | O_APPEND);

    if(file < 0 ){

      perror("eroare la deschiderea fisierlui la logare\n");
      return;
    }

    char log[LOG_SIZE];
    snprintf(log,sizeof(log), "%s\n",message);

    write(file,log,strlen(log));
    close(file);

    char name[FILENAME_SIZE];
    snprintf(name,sizeof(name),"hunt_looged : %s", hunt_id);
    symlink(path,name);

}
 
void adauga_comoara(const char *file_input, const char *hunt_id){

  char file_path[FILENAME_SIZE];
  snprintf(file_path,sizeof(file_path),"%s,%s",hunt_id,"treasures.bin");

  int file = open(file_path, O_WRONLY | O_CREAT | O_APPEND);

    if(file < 0 ){

      perror("eroare la deschiderea fisierlui la adaugare \n");
      return;
    }

    FILE *f_input = fopen(file_input,"r");
    if(f_input){

      perror("eroare de la deschiderea fisierului de input");
      close(file);
      return;
    }

    Treasure t;
    fscanf(f_input,"%d\n",&t.treasure_id);
    fgets(t.username,USERNAME_SIZE,f_input);
    t.username[strcspn(t.username,"\n")] = 0;
    fscanf(f_input,"%f\n",&t.longitude);
    fscanf(f_input,"%f\n",&t.latitude);
    fgets(t.clue,CLUE_SIZE,f_input);
    t.clue[strcspn(t.clue,"\n")] = 0;
    fscanf(f_input,"%d\n",&t.value);

    write(file,&t,sizeof(Treasure));
    close(file);
    fclose(f_input);

    char msg[128];
    snprintf(msg,sizeof(msg),"Treasure %d adaugata cu succes de user %s",t.treasure_id,t.username);
    log_opperation(hunt_id,msg);

}

void lista_treasure(const char *hunt_id){

  char f_input[FILENAME_SIZE];
  snprintf(f_input,sizeof(f_input),"%s,%s",hunt_id,"treasures.bin");

  int file = open(f_input, O_RDONLY);

  if(file < 0 ){

    perror("eroare la deschiderea fisierlui la listare\n");
    return;
  } 
  /*
  struct stat st;
  if(stat(f_input,&st) == 0){
  printf("Hunt : %s\n", hunt_id);
  printf("File size : %ld \n", &st.st_size);
  }*/
  printf("Hunt : %s\n", hunt_id);

  Treasure t;
  while (read(file,&t,sizeof(Treasure)) == sizeof(Treasure))
  { 
    printf("ID : %d, User : %s, Lat : %.2f, Lon : %.2f, Value : %d, Clue : %s \n", t.treasure_id, t.username, t.latitude, t.longitude, t.value, t.clue);
  }

  close(file);
  
}

void view_treasure(const char *hunt_id,int id){

  char f_input[FILENAME_SIZE];
  snprintf(f_input,sizeof(f_input),"%s,%s",hunt_id,"treasures.bin"); 

  int file = open(f_input, O_RDONLY);

  if(file < 0 ){

    perror("eroare la deschiderea fisierlui la view\n");
    return;
  }  

 Treasure t;
  while (read(file,&t,sizeof(Treasure)) == sizeof(Treasure))
  { 
    if(t.treasure_id == id){
    printf("ID : %d, User : %s, Lat : %.2f, Lon : %.2f, Value : %d, Clue : %s \n", t.treasure_id, t.username, t.latitude, t.longitude, t.value, t.clue);
    close(file);
    char msg[128];
    snprintf(msg,sizeof(msg), "Treasure gaist %d", id);
    log_opperation(hunt_id,msg);
    return;
    }
  } 

  printf("Id-ul nu a fost gasit \n");
  close(file);
}

int main(int argc,char **argv){

  if(argc<3){

      perror("Prea putine argumente adaugate \n");
      exit(-1);
  }

  const char *command = argv[1];
  const char *hunt_id = argv[2];

  if(strcmp(command,"--add") == 0 && argc == 4){
    const char *f_input = argv[3];
    adauga_comoara(f_input,hunt_id);
  }else if(strcmp(command,"--list") == 0){
    lista_treasure(hunt_id);
  }else if(strcmp(command,"--view") == 0 && argc == 4){
    view_treasure(hunt_id,atoi(argv[3]));
  }
  else{
    perror("Eroare la comanda data \n");
    return 1;
  }
  return 0;
}