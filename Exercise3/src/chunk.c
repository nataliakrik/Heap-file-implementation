#include <merge.h>
#include <stdio.h>
#include "chunk.h"


CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk){
    CHUNK_Iterator iterator ;
    iterator.current = 0;
    iterator.lastBlocksID = 0;
    iterator.blocksInChunk = blocksInChunk;
    iterator.file_desc = fileDesc;
    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator,CHUNK* chunk){
    int totalBlocks = HP_GetIdOfLastBlock(iterator->file_desc);
    // Calculate the number of chunks to create in case the division of the total blocks and the numBlocksInChunck is not an integer
    int numChunks = (totalBlocks + iterator->blocksInChunk - 1) /iterator->blocksInChunk;

    // Calculating the number of records of the first Block as the rec_per_block stat
    int rec_per_block = HP_GetRecordCounter(iterator->file_desc , 1);
    // Calculating the number of records of the last Block
    int last_block_recs =  HP_GetRecordCounter(iterator->file_desc , totalBlocks);
    /*
    printf("get next :rec_per_block  = %d ,last_block_recs = %d , totalBlocks = %d\n" , rec_per_block , last_block_recs , totalBlocks);
    printf("get next iterator :current = %d , last block = %d\n" , iterator->current , iterator->lastBlocksID);
    */
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->lastBlocksID+1;
    chunk->to_BlockId = chunk->from_BlockId + iterator->blocksInChunk -1;
    // Handle the last chunk, which may have fewer blocks
    if (chunk->to_BlockId >= totalBlocks) {
        // The last chunk's last block will be the last block in the file 
        chunk->to_BlockId = totalBlocks;
        // This (chunk.to_BlockId - chunk.from_BlockId ) is the number of blocks in the last chunk minus the last block so we can calculate correctly the records
        chunk->recordsInChunk = (chunk->to_BlockId - chunk->from_BlockId ) * rec_per_block + last_block_recs;
        chunk->blocksInChunk = chunk->to_BlockId - chunk->from_BlockId + 1;
    }
    else{
        chunk->recordsInChunk = iterator->blocksInChunk * rec_per_block; 
        chunk->blocksInChunk = (chunk->to_BlockId - chunk->from_BlockId + 1);
    }
}



int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){
    int rec_per_block = BF_BLOCK_SIZE / sizeof(Record) , block_id = chunk->from_BlockId + i/rec_per_block ;
    HP_GetRecord(chunk->file_desc , block_id , i/rec_per_block , record);
}

int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){
    int rec_per_block = BF_BLOCK_SIZE / sizeof(Record) , block_id = chunk->from_BlockId + i/rec_per_block ;
    HP_UpdateRecord(chunk->file_desc , block_id , i , record);
}

void CHUNK_Print(CHUNK chunk){

}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk){
    CHUNK_RecordIterator rec_iterator;
    rec_iterator.chunk = *chunk;    
    rec_iterator.currentBlockId = chunk->from_BlockId;
    rec_iterator.cursor = 0;
    return rec_iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record){
    int rec_per_block = BF_BLOCK_SIZE / sizeof(Record) ;
    HP_GetRecord(iterator->chunk.file_desc , iterator->currentBlockId , iterator->cursor/rec_per_block , record);
}
