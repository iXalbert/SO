#ifndef TREASURE_OPERATIONS_H
#define TREASURE_OPERATIONS_H


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


  void adauga_comoara(const char *file_input, const char *hunt_id);
  void lista_treasure(const char *hunt_id);
  void view_treasure(const char *hunt_id, int id);
  void sterge_treasure(const char *hunt_id, int id);
  void sterge_hunt(const char *hunt_id);




#endif