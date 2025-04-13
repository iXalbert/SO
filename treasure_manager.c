#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_operations.h"


int main(int argc,char **argv)
{

  if(argc<3)
  {

      perror("Prea putine argumente adaugate \n");

      printf("Utilizare : \n ");
      printf(" ./treasure_manager --add <Hunt_ID> < treasure_input.txt \n");
      printf(" ./treasure_manager --list <Hunt_ID \n>");
      printf(" ./treasure_manager --view <Hunt_ID> <Treasure_ID>> \n");
      printf(" ./treasure_manager --remove_treasure <Hunt_ID> <Treasure_ID>");
      printf(" ./treasure_manager --remove_hunt <Hunt_ID>");

      exit(-1);
  }

  const char *command = argv[1];
  const char *hunt_id = argv[2];

  if(strcmp(command,"--add") == 0 && argc == 4)
  {
    const char *f_input = argv[3];
    adauga_comoara(f_input,hunt_id);
  }
  else if(strcmp(command,"--list") == 0)
  {
    lista_treasure(hunt_id);
  }
  else if(strcmp(command,"--view") == 0 && argc == 4)
  {
    view_treasure(hunt_id,atoi(argv[3]));
  }
  else if(strcmp(command,"--remove_treasure") == 0 && argc == 4)
  {
    sterge_treasure(hunt_id,atoi(argv[3]));
  }
  else if(strcmp(command,"--remove_hunt") == 0)
  {
    sterge_hunt(hunt_id);
  }
  else
  {
    perror("Eroare la comanda data \n");
    return 1;
  }
  return 0;
}
