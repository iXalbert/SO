#include "treasure_operations.h"
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
#include <errno.h>


void log_opperation(const char *hunt_id,const char *message)
{

    char path[FILENAME_SIZE];
    snprintf(path,sizeof(path),"%s/%s",hunt_id,FILE_INPUT);

    int file = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if(file < 0 )
    {
      perror("eroare la deschiderea fisierlui la logare\n");
      return;
    }

    char log[LOG_SIZE];
    snprintf(log,sizeof(log), "%s\n",message);

    write(file,log,strlen(log));
    close(file);

    printf("Operatia indeplinita cu succes \n ");

    char name[FILENAME_SIZE];
    snprintf(name,sizeof(name),"logged_hunt-%s", hunt_id);
    if(symlink(path,name) < 0 )
    {
        perror("Eroare la creerea legaturi \n");
        exit(-1);
    }


}
 
int file_exist(const char *file)
{
    struct stat buffer;
    return (stat(file, &buffer) == 0);
}

void adauga_comoara(const char *file_input, const char *hunt_id)
{

    if(!(file_exist(file_input)))
    {
        perror("Fisierul nu exista \n");
        exit(-1);  
    }


  mkdir(hunt_id, 0755);

  char file_path[FILENAME_SIZE];
  snprintf(file_path,sizeof(file_path),"%s/%s",hunt_id,"treasures.bin");

  int file = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);

    if(file < 0)
    {
      perror("eroare la deschiderea fisierlui la adaugare \n");
      return;
    }

   int f_input = open(file_input, O_RDONLY);
   if(f_input < 0)
   {
      perror("Eroare la deschidere fisierului la adaugare 2 \n");
      close(file);
      return;
   }

    char buffer[1024];
    ssize_t buffer_read = read(f_input,buffer,sizeof(buffer)-1);


    if(buffer_read < 0)
    {
        perror("Eroare la citirea din fiiser");
        close(file);
        return;
    }

    buffer[buffer_read] = '\0';
    close(f_input);

    Treasure t;

    char *line = strtok(buffer,"\n"); 
    if(!line)
    {
      perror("Format invalid 1 \n");
      close(file);
      return;
    }
    t.treasure_id = atoi(line);

    line = strtok(NULL,"\n");
    if(!line)
    {
      perror("Format invalid 2 \n");
      close(file);
      return;
    }
    strncpy(t.username,line,USERNAME_SIZE);
    t.username[USERNAME_SIZE-1]='\0';

    line = strtok(NULL,"\n");
    if(!line)
    {
      perror("Format invalid 3 \n");
      close(file);
      return;
    }
    t.longitude = atof(line);

    line = strtok(NULL,"\n");
    if(!line)
    {
      perror("Format invalid 4 \n");
      close(file);
      return;
    }
    t.latitude = atof(line);

    line = strtok(NULL,"\n");
    if(!line)
    {
      perror("Format invalid 5 \n");
      close(file);
      return;
    }
    strncpy(t.clue,line,USERNAME_SIZE);
    t.clue[USERNAME_SIZE-1]='\0';

    line = strtok(NULL,"\n");
    if(!line)
    {
      perror("Format invalid 6 \n");
      close(file);
      return;
    }
    t.value = atoi(line);


    /*
    Treasure t;
    fscanf(f_input,"%d\n",&t.treasure_id);
    fgets(t.username,USERNAME_SIZE,f_input);
    t.username[strcspn(t.username,"\n")] = 0;
    fscanf(f_input,"%f\n",&t.longitude);
    fscanf(f_input,"%f\n",&t.latitude);
    fgets(t.clue,CLUE_SIZE,f_input);
    t.clue[strcspn(t.clue,"\n")] = 0;
    fscanf(f_input,"%d\n",&t.value);
    */

    write(file,&t,sizeof(Treasure));
    close(file);

    char msg[128];

    snprintf(msg,sizeof(msg),"Treasure %d adaugata cu succes de user %s",t.treasure_id,t.username);
    log_opperation(hunt_id,msg);

}

void lista_treasure(const char *hunt_id)
{

  char f_input[FILENAME_SIZE];
  snprintf(f_input,sizeof(f_input),"%s/%s",hunt_id,"treasures.bin");

  if(!(file_exist(f_input)))
  {
      perror("Fisierul nu exista \n");
      return;
      //exit(-1);  
  }

  int file = open(f_input, O_RDONLY);

  if(file < 0 )
  {
    perror("eroare la deschiderea fisierlui la listare\n");
    return;
  } 


  struct stat st;

  if(stat(f_input,&st) == 0)
  {
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

void view_treasure(const char *hunt_id,int id)
{

  char f_input[FILENAME_SIZE];
  snprintf(f_input,sizeof(f_input),"%s/%s",hunt_id,"treasures.bin"); 

  if(!(file_exist(f_input)))
  {
      perror("Fisierul nu exista \n");
      return;
      //exit(-1);  
  }

  int file = open(f_input, O_RDONLY);

  if(file < 0 )
  {
    perror("eroare la deschiderea fisierlui la view\n");
    return;
  }  

 Treasure t;

  while (read(file,&t,sizeof(Treasure)) == sizeof(Treasure))
  { 
    if(t.treasure_id == id)
    {
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

void sterge_treasure(const char *hunt_id, int id)
{
  char f_input[FILENAME_SIZE];
  snprintf(f_input,sizeof(f_input),"%s/%s",hunt_id,"treasures.bin");

  if(!(file_exist(f_input)))
  {
      perror("Fisierul nu exista \n");
      exit(-1);  
  }

  int file = open(f_input, O_RDONLY);

  if (file < 0)
  {
    perror("Eroare la deschiderea fisierului de stergere");
    return;
  }

  int f_temp = open("temp.bin", O_WRONLY| O_CREAT | O_TRUNC, 0644);

  if(f_temp < 0)
  {
    perror("Eroare la deschiderea fisierului temporar in stergere");
    close(file);
    return;
  }

  Treasure t;
  int found = 0;

  while (read(file,&t,sizeof(Treasure)) == sizeof(Treasure))
  {
    if (t.treasure_id != id) 
    {
      write(f_temp, &t, sizeof(Treasure));
    } 
    else 
    {
      found = 1;
    }
}

  close(file);
  close(f_temp);
  
  if(found==0)
  {

    printf("Comoara nu a fost gasita\n");
    remove("temp.bin");
    return;
  }

  rename("temp.bin",f_input);

  char msg[128];

  snprintf(msg,sizeof(msg),"Remove trasure %d", id);
  log_opperation(hunt_id,msg);


}

void sterge_hunt(const char *hunt_id)
{

  char f_input[FILENAME_SIZE];

  snprintf(f_input,sizeof(f_input),"%s/%s",hunt_id,"treasures.bin");
  remove(f_input);
  
  snprintf(f_input,sizeof(f_input),"%s/%s",hunt_id,FILE_INPUT);
  remove(f_input);

  rmdir(hunt_id);

  char name_symlink[FILENAME_SIZE];

  snprintf(name_symlink,sizeof(name_symlink),"logged_hunt-%s", hunt_id);
  unlink(name_symlink);

  char msg[128];
  snprintf(msg,sizeof(msg),"Remove hunt %s",hunt_id);
}
