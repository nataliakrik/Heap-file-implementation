#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"


#define rec_per_block (BF_BLOCK_SIZE/sizeof(Record))

/* Determines if two records should be swapped during sorting, returning true
if the order needs adjustment.*/
bool shouldSwap(Record* rec1,Record* rec2){
    int result = strcmp(rec1->name, rec2->name);
    // in case they have the same name     
    if(result == 0)
        result = strcmp(rec1->surname , rec2->surname);
    // If the result is greater than zero, swap the records (adjust the order)
    return result > 0;
}

/* Sorts the contents of a file identified by 'file_desc' in chunks, where
each chunk contains 'numBlocksInChunk' blocks. The sorting is performed
in-place within each chunk, using an appropriate sorting algorithm.*/

void sort_FileInChunks(int file_desc, int numBlocksInChunk){
    // To keep track of the current chunk and overall the file
    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc , numBlocksInChunk) ;

    BF_Block * metadata;
    BF_Block_Init(&metadata);
    BF_GetBlock(file_desc , 0 , metadata);
    void * data = BF_Block_GetData(metadata);
    HP_info *info =(HP_info*)data ;
    
    // Number of blocks in the file considering the metadata blocks id is 0
    int totalBlocks = info->lastBlockId;

    // Calculate the number of chunks to create in case the division of the total blocks and the numBlocksInChunck is not an integer
    int numChunks = (totalBlocks + numBlocksInChunk - 1) / numBlocksInChunk;

    // Calculating the number of records of the last Block
    int last_block_recs = info->totalRecords - (info->lastBlockId-1)*rec_per_block;
    // Iterate over chunks
    for (iterator.current = 0; iterator.current < numChunks; iterator.current++) {
        // Create a CHUNK for the current chunk
        CHUNK chunk;
        chunk.file_desc = file_desc;
        chunk.from_BlockId =iterator.lastBlocksID + 1 ;
        chunk.to_BlockId = chunk.from_BlockId + iterator.blocksInChunk - 1;
        // Handle the last chunk, which may have fewer blocks
        if (chunk.to_BlockId >= totalBlocks) {
            // The last chunk's last block will be the last block in the file 
            chunk.to_BlockId = totalBlocks;
            // This (chunk.to_BlockId - chunk.from_BlockId ) is the number of blocks in the last chunk minus the last block so we can calculate correctly the records
            chunk.recordsInChunk = (chunk.to_BlockId - chunk.from_BlockId ) * rec_per_block + last_block_recs;
            chunk.blocksInChunk = chunk.to_BlockId - chunk.from_BlockId + 1;
        }
        else{
            chunk.recordsInChunk = iterator.blocksInChunk * rec_per_block; 
            chunk.blocksInChunk = (chunk.to_BlockId - chunk.from_BlockId + 1);
        }
        // Update the iterator to the last block
        iterator.lastBlocksID = chunk.to_BlockId;
        // Sort the chunk that was created
        sort_Chunk(&chunk);
    }

}



/* Sorts records within a CHUNK in ascending order based on the name and
surname of each person. */
void sort_Chunk(CHUNK* chunk){
    int i, j , checks , old_cursor , old_block  ;

    // An iterator through the chunk to keep track of the block and the records in them
    CHUNK_RecordIterator rec_iterator;
    Record * currentRecord = (Record *)malloc(sizeof(Record))  , * nextRecord = (Record *)malloc(sizeof(Record)) ;
    rec_iterator = CHUNK_CreateRecordIterator(chunk);

    bool change_block = false;
    // Creating a block struct to get the records
    BF_Block * block;
    BF_Block_Init(&block);
    BF_GetBlock(chunk->file_desc , rec_iterator.currentBlockId , block);

    // Creating a variable to know if this is the last chunk which means that has less records than expected
    bool last_chunk= false;
    if (chunk->recordsInChunk<chunk->blocksInChunk * rec_per_block){
        last_chunk=true;
    }
    for (i = 0; i < chunk->recordsInChunk - 1; i++) {
        rec_iterator.cursor = 0;
        rec_iterator.currentBlockId = chunk->from_BlockId; 
        for (j = 0; j < chunk->recordsInChunk - i - 1; j++) {
            HP_GetRecord(chunk->file_desc , rec_iterator.currentBlockId ,rec_iterator.cursor , currentRecord );
            // it hits the end of the current block which means that the new record belongs to the new block
            if (rec_iterator.cursor  == rec_per_block - 1){
                old_cursor = rec_iterator.cursor;
                old_block = rec_iterator.currentBlockId;
                rec_iterator.cursor = 0;
                if(rec_iterator.currentBlockId == chunk->to_BlockId)
                    rec_iterator.currentBlockId = chunk->from_BlockId;
                else
                    rec_iterator.currentBlockId ++;
                change_block = true;
                HP_GetRecord(chunk->file_desc , rec_iterator.currentBlockId , rec_iterator.cursor , nextRecord );
            }
            // Check if its the last block of the last chunk and it has less records than all the other blocks
            else if(last_chunk==true && rec_iterator.cursor == (chunk->blocksInChunk*rec_per_block)-chunk->recordsInChunk  && rec_iterator.currentBlockId==chunk->to_BlockId){
                old_cursor = rec_iterator.cursor;
                old_block = rec_iterator.currentBlockId;
                rec_iterator.cursor = 0;
                //printf("\ncurrent block = %d , \n" , rec_iterator.currentBlockId);
                rec_iterator.currentBlockId = chunk->from_BlockId;
                //printf("\nnew block = %d , \n" , rec_iterator.currentBlockId);
                change_block = true;
                HP_GetRecord(chunk->file_desc , rec_iterator.currentBlockId , rec_iterator.cursor , nextRecord );
            }
            else
                HP_GetRecord(chunk->file_desc , rec_iterator.currentBlockId , (rec_iterator.cursor + 1) , nextRecord );
        
            if (shouldSwap(currentRecord, nextRecord)) {
                // Swap the records
                if (change_block){
                    HP_UpdateRecord(chunk->file_desc , old_block , old_cursor , *nextRecord);
                    HP_UpdateRecord(chunk->file_desc , rec_iterator.currentBlockId , 0 , *currentRecord);
                }
                else{
                    HP_UpdateRecord(chunk->file_desc , rec_iterator.currentBlockId , rec_iterator.cursor , *nextRecord);
                    HP_UpdateRecord(chunk->file_desc , rec_iterator.currentBlockId , rec_iterator.cursor + 1 , *currentRecord);
                }
            }
            rec_iterator.cursor ++;
            
            // Save the data of the last block that we made changes to
            if(change_block){
                BF_Block_SetDirty(block);
                BF_UnpinBlock(block);
                BF_GetBlock(chunk->file_desc , rec_iterator.currentBlockId , block);
                change_block = false;
                rec_iterator.cursor = 0;
            }
        }
    }
    free(currentRecord);
    free(nextRecord);
}