#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}
////////////////////
//  Συναρτηση hashing για την τοποθετηση εγγραφων στα buckets
int hash_function(int record_id , int num_buckets){
  return (record_id%num_buckets)+1;
}

// Μια δομη που περιεχει πληροφοριες για το hash table
typedef struct {
    int bucket_num ;
    int file_desc ;
    int global_depth;
    BF_Block *first_bucket;
} HT_info;

typedef struct check_buckets{
  int bucket_id;
  struct check_buckets * next_bucket;
}check_buckets;

// Δομη που αποτελει τα buckets
typedef struct{
    int records ;
    int bucket_id;
    int local_depth;
    check_buckets * connected_buckets;
    BF_Block *next_bucket ; 
} HT_bucket_info;

// hash table ως πινακας που περιλαμβανει αποτελειται απο τα μεγιστα ανοιχτα hash table που μπορουμε να εχουμε
HT_info *hash_table[MAX_OPEN_FILES];

// Αριθμος απο εγγραφες που χωρανε σε καθε bucket
#define rec_per_bucket (BF_BLOCK_SIZE-sizeof(HT_bucket_info))/sizeof(Record) 

// Counter για τα ανοιχτα αρχεια
int open_tables=0;

HT_ErrorCode HT_Init() {

  for (int i = 0; i < MAX_OPEN_FILES; i++)
    hash_table[i] = NULL;
  
  return HT_OK;
}


HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
/*
Η συνάρτηση HT_CreateIndex χρησιμοποιείται για τη δημιουργία και την κατάλληλη
αρχικοποίηση ενός άδειου αρχείου επεκτατού κατακερματισμού με όνομα fileName. Σε
περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική σε
περίπτωση κωδικός λάθους.
*/

  // Δημιουργια αρχειου
  int file_desc;
  CALL_BF(BF_CreateFile(filename));
  CALL_BF(BF_OpenFile(filename , &file_desc));         

  // Δημιουργια δομης του αρχειου και των πληροφοριων του
  HT_info * h_table;

  // Δημιουργια του block που περιλεμβανει μεταδεδομενα του αρχειου
  BF_Block *block;
  BF_Block_Init(&block);
  BF_AllocateBlock(file_desc , block);
  void * data;
  data =BF_Block_GetData(block);
  
  h_table = (HT_info *)data;
  h_table->file_desc=file_desc;
  h_table->global_depth = depth;
  h_table->first_bucket = NULL ;
  h_table->bucket_num = 0;
  
  // Κανουμε το μπλοκ dirty και unpin
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_CloseFile(file_desc);
  return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
/*
Η ρουτίνα αυτή ανοίγει το αρχείο με όνομα fileName. Εάν το αρχείο ανοιχτεί κανονικά, η
ρουτίνα επιστρέφει HT_OK. Σε διαφορετική περίπτωση, επιστρέφει HT_ERROR.
*/

/*
Ανοιγουμε το αρχειο δημιουργουμε εναν δεικτη στα μεταδεδομενα του αρχειο .
Αποθηκευουμε αυτον τον δεικτη στον πινακα με τα ανοιχτα αρχεια ως το τελευταιο ανοιχτο αρχειο
με counter το opentables και στην συνεχεια αλλαζουμε τον δεικτη.
Επισησ δημιουργουμε το πρωτο αδειο μπλοκ και το τοποθετουμε στην θεση first bucket 
στα μεταδεδομενα του αρχειου.
Στην συνεχεια προσθετουμε στο μπλοκ την δομη με της πληροφοριες του bucket.
*/
  if (open_tables == MAX_OPEN_FILES){
    printf("\nWe reached maximum open files. Cannot open more.\n");
    return HT_ERROR;
  }
  else
    printf("\nFile can be opened\n");

  BF_Block *metadata ,* bucket;
  HT_info *metadata_pointer;
  CALL_BF(BF_OpenFile(fileName , indexDesc));
  

// Μεταδεδομενα του αρχειου
  BF_Block_Init(&metadata);
  BF_GetBlock(*indexDesc, 0 , metadata );
  void *data;
  data = BF_Block_GetData(metadata);
  metadata_pointer = (HT_info*)data;
  
  data = NULL;
// Δημιουργια και αρχικοποιηση του πρωτου bucket με 0 εγγραφες
  HT_bucket_info *bucket_info;
  BF_Block_Init(&bucket);
  BF_AllocateBlock(*indexDesc , bucket);

// Συνδεση του πρωτου bucket με τα μεταδεδομενα του hash table
  metadata_pointer->first_bucket=bucket;
  metadata_pointer->bucket_num=1;

  hash_table[open_tables] = metadata_pointer;
  open_tables++;

  data = BF_Block_GetData(bucket);
  bucket_info = (HT_bucket_info*)data;
  bucket_info->bucket_id = (metadata_pointer->bucket_num);
  bucket_info->local_depth=(metadata_pointer->global_depth);
  bucket_info->next_bucket = NULL;
  bucket_info->records=0;
  bucket_info->connected_buckets = NULL;

  BF_Block_SetDirty(metadata);
  CALL_BF(BF_UnpinBlock(metadata));
  BF_Block_SetDirty(bucket);
  CALL_BF(BF_UnpinBlock(bucket));
  printf("\nOpened table with indexDesc = %d there are %d opentables\n" , *indexDesc , open_tables);
  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
/*
Η ρουτίνα αυτή κλείνει το αρχείο επεκτατού κατακερματισμού του οποίου οι πληροφορίες
βρίσκονται στην θέση indexDesc του πίνακα ανοιχτών αρχείων. Επίσης σβήνει την
καταχώρηση που αντιστοιχεί στο αρχείο αυτό στον πίνακα ανοιχτών αρχείων. Η συνάρτηση
επιστρέφει EH_OK εάν το αρχείο κλείσει επιτυχώς, ενώ σε διαφορετική σε περίπτωση
κωδικός λάθους
*/
  if (open_tables == 0){
    printf("\nThere are no open files\n");
    return HT_ERROR;
  }
  HT_info *file;
  int i;
  for (i = 0; i < open_tables; i++)
  {
    file=hash_table[i];
    if (file->file_desc == indexDesc){
      hash_table[i] = NULL;
      break;
    }
  }
  if(i == open_tables){
    printf("\nThere is no open hash table with that indexDesc\n");
    return HT_ERROR;
  }
  printf("\nThe table with indexDesc = %d will be closed\n" , indexDesc);
  free(file);
  open_tables--;
  return HT_OK;
}



HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
/*
Η συνάρτηση HT_InsertEntry χρησιμοποιείται για την εισαγωγή μιας εγγραφής στο αρχείο
κατακερματισμού. Οι πληροφορίες που αφορούν το αρχείο βρίσκονται στον πίνακα ανοιχτών
αρχείων, ενώ η εγγραφή προς εισαγωγή προσδιορίζεται από τη δομή record. Σε περίπτωση
που εκτελεστεί επιτυχώς επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κάποιος
κωδικός λάθους.

*/

  HT_info *index;
  for (int i = 0; i < open_tables; i++)
  {
    index = hash_table[i];
    if(index->file_desc == indexDesc)
      break;
  }
  int hash_value = hash_function(record.id , index->bucket_num);
  BF_Block *bucket;
  BF_Block_Init(&bucket);
  BF_GetBlock(indexDesc , hash_value ,bucket); 

  HT_bucket_info *bucket_info;
  void *data;
  data = BF_Block_GetData(bucket);

  /*
   Υπαρχουν και οι δυο περιπτωσεις οπου μπορει να υπαρχει το συγκεκριμενο μπλοκ 
   αλλα μπορει και  να μην υπαρχει.
   Πρωτα θα εξεταστει η περιπτωση που υπαρχει.
  */
  if(data != NULL){
    bucket_info = (HT_bucket_info*)data;
  
    if ( bucket_info->records < rec_per_bucket ){
      /*
      Σε 1η περιπτωση θα κοιταξω αν υπαρχει διαθεσιμος χωρος στο bucket 
      που θελω να τοποθετησω την εγγραφη
      Ας πουμε οτι υπαρχει, δλδ θα παω στην δομη info του μπλοκ και θα συγκρινω records< rec_per_bucket
      οπου records ειναι οι εγγραφες που περιλαμβανονται μεσα στο bucket μεχρις στιγμης και rec_per_bucket 
      ειναι σταθερα με ποσες εγγραφες χωρανε μεσα στο bucket
      */
      data = data + sizeof(HT_bucket_info);
      memcpy(data , &record , sizeof(Record));
      (bucket_info->records)++;
    } 
    /////////////////////////// Τελος 1ησ περιπτωησης///////////////////////////////
    else if(bucket_info->local_depth < index->global_depth){
      /*
      2η περιπτωση θα χρειαστει να επανατοποθετησουμε τις εγγραφες 
      του bucket που εχουμε ανοιξει.Δηλαδη καλουμε την συναρτηση hashing 
      για καθε μια απο τις εγγραφες για να βρουμε την καινουργια της θεση
      */
      
      // Επαναπροσδιορισμος των πληροφοριων του παλιου bucket

      bucket_info->records = 0;
      printf("\nThere is no space in the current bucket and the local depth is less than the global \n we will rehash the all records of this bucket\n");
      data = data + sizeof(HT_bucket_info);
      Record *rec;
      int hashing_value;
      void * old_data = data;

      // επανατοποθετηση των εγγραφων 
      for(int i = 0; i< rec_per_bucket; i++){
        rec = (Record *)data;
        // Αν το hasing_value δεν ανηκει το bucket τοτε θα ανηκει σε ενα απο τα connected
        hashing_value = hash_function(rec->id , index->bucket_num );
        if(hashing_value == bucket_info->bucket_id){
          memcpy(old_data , rec, sizeof(Record));
          (bucket_info->records)++;
          old_data = old_data + sizeof(Record);
          break;
        }
        else{
          // Ελεγχω τους "δεικτες" που δειχνουν στο αρχικο bucket και δημιουργω ενα καινουργο 
          // bucket αν η συγκεκριμενη εγγραφη πρεπει να τοποθετηθει σε καινουργιο
          check_buckets * connected = bucket_info->connected_buckets;
          while(connected!= NULL){
          
            if (hashing_value == connected->bucket_id){

              // Δημιουργουμε το καινουργιο bucket 
              HT_bucket_info *new_bucket_info;
              BF_Block * new_bucket;
              BF_Block_Init(&new_bucket);
              void * new_data;
              BF_GetBlock(indexDesc , connected->bucket_id , new_bucket);
              BF_AllocateBlock(indexDesc , new_bucket);

              new_data = BF_Block_GetData(new_bucket);
              if (new_data == NULL){
                printf("\nCreating new bucket for the record\n");
                BF_AllocateBlock(indexDesc , new_bucket);
                new_data = BF_Block_GetData(bucket);
                new_bucket_info = (HT_bucket_info *)new_data;
                new_bucket_info->bucket_id = connected->bucket_id;
                new_bucket_info->next_bucket = bucket_info->next_bucket;
                new_bucket_info->connected_buckets = NULL;
                new_bucket_info->records = 0;
                // Επαναπροσδιορισμος των πληροφοριων του παλιου bucket ετσι ωστε να δειχνει στο καινουργιο
                // και το καινουργιο δειχνει εκει που εδεχνε το παλιο
                bucket_info->next_bucket = new_bucket;
                // Αυξηση το local_depth καθως αφαιρουμε ενα connected bucket αφου το δημιουργησαμε
                (bucket_info->local_depth)++;
                (index->bucket_num)++;
              }
              
              // Τοποθετηση της εγγραφης στο bucket και εξοδος απο την επαναληψη για ελεγχο της επομενη εντολης
              new_bucket_info = (HT_bucket_info *)new_data;
              new_bucket_info->local_depth = (bucket_info->local_depth)++;
              (new_bucket_info->records)++;
              
              memcpy(new_data , rec, sizeof(Record));
              printf("\nRecord added succesfully to the new bucket\n");
              BF_Block_SetDirty(new_bucket);
              CALL_BF(BF_UnpinBlock(new_bucket));
              break;
            }
            connected=connected->next_bucket;
          }
          printf("\nChecking next record");
          data = data + sizeof(Record);
        }
      }

      if(bucket_info->local_depth == index->global_depth){
        printf("\nThe buckets depth is global depth which means that there is no connected buckets\nSo removing pointer to connected buckets\n");
        bucket_info->connected_buckets = NULL;
      }
      // Αφου εχουμε επανασχεδιασει το hash table καλουμε την insert entry για την σωστη τοποθετηση τησ καινουργιας εγγραφης
      HT_InsertEntry(indexDesc , record);
      BF_Block_SetDirty(bucket);
      CALL_BF(BF_UnpinBlock(bucket));
    }
    //////////////// Τελος 2ης περιπτωησης///////////////////
    else{
      //////////////3η περιπτωση////////////////////////////
      /*
      3η περιπτωση δεν υπαρχει χωρος και χρειαζεται να διπλασιασουμε το hash table
      και να τοποθετησουμε τα connected buckets id στα μισα τους αν υπαρχει bucket 
      αλλιως στα μισα αν υπαρχει bucket...κλπ

      γνωριζουμε οτι υπαρχει το bucket αλλα πρεπει να:
      - δημιουργησουμε καινουργο bucket το οποιο εχει id
      - να αυξησουμε το global depth
      - να επαναπροδιορισουμε τις θεσεις των εγγραφων
      - να διπλασιασουμε το μεγεθος των buckets 
      */
      printf("\nThe hash table will be doubled and the records will be rehashed\n");

      // Συνδεση του καθε καινουργιου bucket με το πρηγουμενο που πρεπει να συνδεθει
      // Το νεο id θα ειναι στην θεση = παλιο_id + το αριθμο απο buckets πριν την αλλαγη
      int new_bucket_id = index->bucket_num;

      for (int i = 1; i <= index->bucket_num; i++){

        BF_Block * changed_bucket;
        BF_Block_Init(&changed_bucket);
        void * change_data;
        BF_GetBlock(indexDesc , i , changed_bucket);
        change_data = BF_Block_GetData(changed_bucket);

        if (change_data == NULL){
          int id = i;
          while (change_data != NULL){
            id = id /2;
            BF_GetBlock(indexDesc , id , changed_bucket);
            change_data = BF_Block_GetData(changed_bucket);
            if (id == 0){
              printf("\nError the connected bucket was not found \n");
              return HT_ERROR;
            }
          }
        }
        HT_bucket_info * pointer;
        pointer = (HT_bucket_info *)change_data;
        check_buckets * pointer1= pointer->connected_buckets;
        
        while (pointer1 != NULL){
          pointer1=pointer1->next_bucket;
        }
        pointer1->bucket_id = new_bucket_id + i;
        pointer1->next_bucket = NULL;

      }
      
      // Τελος με την επαναδιαταξη των συνδεσεων μεταξυ καινουργιου και παλιου bucket

      (index->global_depth)++;
      index->bucket_num = (index->bucket_num) * 2;

      // Δημιουργια του κανουργιου μπλοκ
      new_bucket_id = index->bucket_num + bucket_info->bucket_id;
      HT_bucket_info *new_bucket_info;
      BF_Block * new_bucket;
      BF_Block_Init(&new_bucket);
      BF_AllocateBlock(indexDesc , new_bucket);
      void *new_data;
      new_data = BF_Block_GetData(new_bucket);
      new_bucket_info = (HT_bucket_info*)new_data;

      new_bucket_info->bucket_id = new_bucket_id;
      new_bucket_info->connected_buckets = NULL;
      new_bucket_info->local_depth = index->global_depth;
      new_bucket_info->records = 0;
      new_bucket_info->next_bucket = bucket_info->next_bucket;

      bucket_info->next_bucket = new_bucket;
      bucket_info->local_depth = index->global_depth;
      bucket_info->records = 0;

      // Αφου αρχικοποιησαμε , δημιουργησαμε το καινουργιο bucket και εγιναν αλλαγες στο παλιο 
      // επανατοποθετουμε τις εγγραφες
      printf("\nCreated new bucket and changed info of the old bucket.\nStarting calculate the new hash values of th old recs\n");

      data = data + sizeof(HT_bucket_info);
      Record *rec;
      int hashing_value;
      void * old_data = data;

      for (int i = 0; i < rec_per_bucket; i++){
        rec = (Record *)data;
        hashing_value = hash_function(rec->id , index->bucket_num );
        if (hashing_value == bucket_info->bucket_id){
          memcpy(old_data , rec , sizeof(Record));
          old_data = old_data + sizeof(Record);
          (bucket_info->records)++;
          break;
        }
        else{
          memcpy(new_data , rec , sizeof(Record));
          new_data = new_data + sizeof(Record);
          (new_bucket_info->records)++;
          break;
        }
        data = data + sizeof(Record);
      }
      // Αφου εχουμε επανασχεδιασει το hash table καλουμε την insert entry για την σωστη τοποθετηση τησ καινουργιας εγγραφης
      (index->bucket_num)++;
      HT_InsertEntry(indexDesc , record);
      BF_Block_SetDirty(bucket);
      CALL_BF(BF_UnpinBlock(bucket));
      BF_Block_SetDirty(new_bucket);
      CALL_BF(BF_UnpinBlock(new_bucket));
    
    }
    /////////////  τελος 3ης περιπτωσης///////////////////
  }
  else{
    ///////////////4η περιπτωση /////////////////////
    /*
    Σε αυτην περιπτωση το μπλοκ στο οποιο η εγραφη πρεπει να τοποθετηθει δεν υπαρχει
    οποτε  πρεπει να βρουμε το μπλοκ με το οποιο συνδεεται και να εφαρμοσουμε την περιπτωση 2

    Επομενως πρωτα θα βρεθει το bucket που ψαχνουμε που συνδεεται με το searching_id
    */
    printf("\nThe bucket we are searching has not been implemented yet\nSearching for connected bucket\n");
    int searching_id = hash_value ;
    int id = searching_id;
    void * connected_data= data;
    BF_Block * connected_bucket;
    BF_Block_Init(&connected_bucket);

    while (connected_data != NULL){
      id = id /2;
      BF_GetBlock(indexDesc , id , connected_bucket);
      connected_data = BF_Block_GetData(connected_bucket);
      if (id == 0){
        printf("\nError the connected bucket was not found \n");
        return HT_ERROR;
      }
    }
    
    HT_bucket_info *connected_bucket_info;

    // πρεπει να εφαρμοσουμε την περιπτωση 2 στο connected bucket
    connected_bucket_info->records = 0;
    printf("\nThere is no space in the current bucket and the local depth is less than the global \n we will rehash the all records of this bucket\n");
    connected_data = connected_data + sizeof(HT_bucket_info);
    Record *rec;
    int hashing_value;
    void * old_data = connected_data;

    // επανατοποθετηση των εγγραφων 
    for(int i = 0; i< rec_per_bucket; i++){
      rec = (Record *)connected_data;
      // Αν το hasing_value δεν ανηκει το bucket τοτε θα ανηκει σε ενα απο τα connected
      hashing_value = hash_function(rec->id , index->bucket_num );
      if(hashing_value == connected_bucket_info->bucket_id){
        memcpy(old_data , rec, sizeof(Record));
        (connected_bucket_info->records)++;
        old_data = old_data + sizeof(Record);
        break;
      }
      else{
        // Ελεγχω τους "δεικτες" που δειχνουν στο αρχικο bucket και δημιουργω ενα καινουργο 
        // bucket αν η συγκεκριμενη εγγραφη πρεπει να τοποθετηθει σε καινουργιο
        check_buckets * connected = connected_bucket_info->connected_buckets;
        while(connected!= NULL){
        
          if (hashing_value == connected->bucket_id){

            // Δημιουργουμε το καινουργιο bucket 
            HT_bucket_info *new_bucket_info;
            BF_Block * new_bucket;
            BF_Block_Init(&new_bucket);
            void * new_data;
            BF_GetBlock(indexDesc , connected->bucket_id , new_bucket);
            BF_AllocateBlock(indexDesc , new_bucket);

            new_data = BF_Block_GetData(new_bucket);
            if (new_data == NULL){
              printf("\nCreating new bucket for the record\n");
              BF_AllocateBlock(indexDesc , new_bucket);
              new_data = BF_Block_GetData(bucket);
              new_bucket_info = (HT_bucket_info *)new_data;
              new_bucket_info->bucket_id = connected->bucket_id;
              new_bucket_info->next_bucket = bucket_info->next_bucket;
              new_bucket_info->connected_buckets = NULL;
              new_bucket_info->records = 0;
              // Επαναπροσδιορισμος των πληροφοριων του παλιου bucket ετσι ωστε να δειχνει στο καινουργιο
              // και το καινουργιο δειχνει εκει που εδεχνε το παλιο
              connected_bucket_info->next_bucket = new_bucket;
              // Αυξηση το local_depth καθως αφαιρουμε ενα connected bucket αφου το δημιουργησαμε
              (connected_bucket_info->local_depth)++;
              (index->bucket_num)++;
            }
            
            // Τοποθετηση της εγγραφης στο bucket και εξοδος απο την επαναληψη για ελεγχο της επομενη εντολης
            new_bucket_info = (HT_bucket_info *)new_data;
            new_bucket_info->local_depth = (bucket_info->local_depth)++;
            (new_bucket_info->records)++;
            
            memcpy(new_data , rec, sizeof(Record));
            printf("\nRecord added succesfully to the new bucket\n");
            BF_Block_SetDirty(new_bucket);
            CALL_BF(BF_UnpinBlock(new_bucket));
            break;
          }
          connected=connected->next_bucket;
        }
        printf("\nChecking next record");
        connected_data = connected_data + sizeof(Record);
      }
    }

    if(connected_bucket_info->local_depth == index->global_depth){
      printf("\nThe buckets depth is global depth which means that there is no connected buckets\nSo removing pointer to connected buckets\n");
      connected_bucket_info->connected_buckets = NULL;
    }
    // Αφου εχουμε επανασχεδιασει το hash table καλουμε την insert entry για την σωστη τοποθετηση τησ καινουργιας εγγραφης
    HT_InsertEntry(indexDesc , record);
    BF_Block_SetDirty(bucket);
    CALL_BF(BF_UnpinBlock(bucket));
    BF_Block_SetDirty(connected_bucket);
    CALL_BF(BF_UnpinBlock(connected_bucket));

  }
  ////////////////  τελος 4η περιπτωσης   //////////////////
  return HT_OK;
}
/////////////////////// set_dirty , unpin /////////////////////////////////////////////



HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  return HT_OK;
}