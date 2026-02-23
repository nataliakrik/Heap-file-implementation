#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"

#define RECORDS_NUM 600// you can change it if you want
#define FILE_NAME "data.db"


#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }



int main() {
  BF_Init(LRU);

  HP_CreateFile(FILE_NAME);
  int file_desc;

  HP_info* hp_info2=HP_OpenFile(FILE_NAME, &file_desc);
  Record record;
  srand(time(0));
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    //δημιουργια τυχαιου id για την εγγραφη που εισαγεται 
    int random_id = 1+rand() % RECORDS_NUM ;
    record = randomRecord();
    //αλλαγη του id που εχει ηδη η εγγραφη σε ενα τυχαιο
    record.id = random_id;
    HP_InsertEntry(file_desc,hp_info2, record);
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n",id);
  HP_GetAllEntries(file_desc,hp_info2, id);

  //κλεισιμο του αρχειου και αποδεσμευση μνημης για τα μεταδεδομενα του 
  if (HP_CloseFile(file_desc, hp_info2) == 0)
    printf("File closed successfully.\n");
  else
    printf("Error closing the file.\n");
  

  // Διαγραφη του αρχειου που δημιουργηθηκε
  if (remove(FILE_NAME) == 0)
    printf("File deleted successfully.\n");
  else
    printf("Error deleting the file\n");
  
  BF_Close();
}
