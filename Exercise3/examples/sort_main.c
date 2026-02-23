#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merge.h"

#define RECORDS_NUM 500 // you can change it if you want
#define FILE_NAME "data.db"
#define OUT_NAME "out.db"




int createAndPopulateHeapFile(char* filename);

void sortPhase(int file_desc,int chunkSize);

void mergePhases(int inputFileDesc,int chunkSize,int bWay, int* fileCounter);

int nextOutputFile(int* fileCounter);

int main() {
  int chunkSize=5;
  int bWay= 3;
  int fileIterator;
  //
  BF_Init(LRU);
  int file_desc = createAndPopulateHeapFile(FILE_NAME);
  //HP_PrintAllEntries(file_desc);

  sortPhase(file_desc,chunkSize);
  HP_PrintAllEntries(file_desc);

  mergePhases(file_desc,chunkSize,bWay,&fileIterator);

}

int createAndPopulateHeapFile(char* filename){
  HP_CreateFile(filename);
  
  int file_desc;
  HP_OpenFile(filename, &file_desc);
  Record record;
  srand(12569874);
  for (int id = 0; id < RECORDS_NUM; ++id)
  {
    record = randomRecord();
    HP_InsertEntry(file_desc, record);
  }
  // Adding info to the metadata of the file
  BF_Block * metadata;
  BF_Block_Init(&metadata);
  BF_GetBlock(file_desc , 0 , metadata);
  void * data = BF_Block_GetData(metadata);
  // initialize metadata of the file
  HP_info *info =(HP_info*)data ;
  info->totalRecords = RECORDS_NUM;
  info->lastBlockId =  (RECORDS_NUM + info->blockCapacity-1)/info->blockCapacity ;
  return file_desc;
}

/*Performs the sorting phase of external merge sort algorithm on a file specified by 'file_desc', using chunks of size 'chunkSize'*/
void sortPhase(int file_desc,int chunkSize){ 
  sort_FileInChunks( file_desc, chunkSize);
}

/* Performs the merge phase of the external merge sort algorithm  using chunks of size 'chunkSize' and 'bWay' merging. The merge phase may be performed in more than one cycles.*/
void mergePhases(int inputFileDesc,int chunkSize,int bWay, int* fileCounter){
  int oututFileDesc;
  BF_CreateFile(OUT_NAME);
  BF_PrintError(BF_OpenFile(OUT_NAME , &oututFileDesc));  

  BF_Block *metadata ,* old_info;
  BF_Block_Init(&metadata);
  BF_Block_Init(&old_info);
  BF_AllocateBlock(oututFileDesc , metadata);
  BF_GetBlock(inputFileDesc , 0, old_info);
  HP_info * info = (HP_info *)BF_Block_GetData(metadata)  , *old_info1 = (HP_info *)BF_Block_GetData(old_info);
  info->totalRecords = old_info1->totalRecords;
  info->lastBlockId =  old_info1->lastBlockId;
  info->blockCapacity = old_info1->blockCapacity;
  BF_Block_SetDirty(metadata);
  BF_UnpinBlock(metadata);
  merge(inputFileDesc, chunkSize, bWay, oututFileDesc );
  
  while(chunkSize<=HP_GetIdOfLastBlock(inputFileDesc)){
    oututFileDesc =   nextOutputFile(fileCounter);
    merge(inputFileDesc, chunkSize, bWay, oututFileDesc );
    HP_CloseFile(inputFileDesc);
    chunkSize*=bWay;
    inputFileDesc = oututFileDesc;
  }
  //HP_CloseFile(oututFileDesc);
}

/*Creates a sequence of heap files: out0.db, out1.db, ... and returns for each heap file its corresponding file descriptor. */
int nextOutputFile(int* fileCounter){
    char mergedFile[50];
    char tmp[] = "out";
    sprintf(mergedFile, "%s%d.db", tmp, (*fileCounter)++);
    int file_desc;
    HP_CreateFile(mergedFile);
    HP_OpenFile(mergedFile, &file_desc);
    return file_desc;
}
