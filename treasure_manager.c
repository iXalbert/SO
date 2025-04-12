#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

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

    int file = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);

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

  mkdir(hunt_id, 0755);

  char file_path[FILENAME_SIZE];
  snprintf(file_path,sizeof(file_path),"%s,%s",hunt_id,"treasures.bin");

  int file = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if(file < 0){
      perror("eroare la deschiderea fisierlui la adaugare \n");
      return;
    }

    FILE *f_input = fopen(file_input, "r");
    if (!f_input) {
        perror("fopen input file");
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


  struct stat st;
  if(stat(f_input,&st) == 0){
  printf("Hunt : %s\n", hunt_id);
  printf("File size : %lld \n", st.st_size);
  printf("Last modified : %s", ctime(&st.st_mtime));
  }

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

    char msg[128];
    snprintf(msg,sizeof(msg), "Treasure gaist %d", id);
    log_opperation(hunt_id,msg);
    close(file);
    return;
    }
  } 

  printf("Id-ul nu a fost gasit \n");
  close(file);
}

void sterge_treasure(const char *hunt_id, int id){
  char f_input[FILENAME_SIZE];
<<<<<<< HEAD
  snprintf(f_input,sizeof(f_input),"%s,%s",hunt_id,"treasures.bin");

  int file = open(f_input, O_RDONLY);
  if (file < 0){
=======
  snprintf(f_input,sizeof(f_input),"%s , %s",hunt_id,"treasures.bin");

  int f;
  if ((f = open(f_input, O_RDONLY)) < 0){
>>>>>>> b080797dbff3bd813e6b1230806f53ae5d5125a4
    perror("Eroare la deschiderea fisierului de stergere");
    return;
  }

  int f_temp = open("temp.bin", O_WRONLY| O_CREAT | O_TRUNC, 0644);
  if(f_temp < 0){
    perror("Eroare la deschiderea fisierului temporar in stergere");
<<<<<<< HEAD
    close(file);
=======
    close(f);
>>>>>>> b080797dbff3bd813e6b1230806f53ae5d5125a4
    return;
  }

  Treasure t;
  int found = 0;
<<<<<<< HEAD
  while (read(file,&t,sizeof(Treasure)) == sizeof(Treasure)){
=======
  while (read(f,&t,sizeof(Treasure)) == sizeof(Treasure)){
>>>>>>> b080797dbff3bd813e6b1230806f53ae5d5125a4
    if (t.treasure_id != id) {
      write(f_temp, &t, sizeof(Treasure));
  } else {
      found = 1;
  }
}
<<<<<<< HEAD
  close(file);
=======
  close(f);
>>>>>>> b080797dbff3bd813e6b1230806f53ae5d5125a4
  close(f_temp);
  
  if(found==0){

    printf("Comoara nu a fost gasita\n");
    remove("temp.bin");
    return;
  }

  rename("temp.bin",f_input);
  char msg[128];
  snprintf(msg,sizeof(msg),"Remove trasure %d", id);
  log_opperation(hunt_id,msg);

<<<<<<< HEAD

=======
  close(f);
  close(f_temp);
>>>>>>> b080797dbff3bd813e6b1230806f53ae5d5125a4
}

void stergere_hunt(const char *hunt_id){

  char f_input[FILENAME_SIZE];
  snprintf(f_input,sizeof(f_input),"%s,%s",hunt_id,"treasures.bin");
  remove(f_input);
  
  snprintf(f_input,sizeof(f_input),"%s,%s",hunt_id,FILE_INPUT);
  remove(f_input);

  rmdir(hunt_id);

  char msg[128];
  snprintf(msg,sizeof(msg),"Remove hunt %s",hunt_id);
  log_opperation(hunt_id,msg);
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
<<<<<<< HEAD
  }else if(strcmp(command,"--remove_treasure") == 0 && argc == 4){
    sterge_treasure(hunt_id,atoi(argv[3]));
  }else if(strcmp(command,"--remove_hunt") == 0){
=======
  }else if(strcmp(command,"--remove_treasure") == 0 && argc == 5){
    sterge_treasure(hunt_id,atoi(argv[4]));
  }else if(strcmp(command,"--remove_hunt") == 0 && argc == 4){
>>>>>>> b080797dbff3bd813e6b1230806f53ae5d5125a4
    stergere_hunt(hunt_id);
  }
  else{
    perror("Eroare la comanda data \n");
    return 1;
  }
  return 0;
}
