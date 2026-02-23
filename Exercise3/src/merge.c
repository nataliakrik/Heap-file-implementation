#include <merge.h>
#include <stdio.h>
#include <stdbool.h>


/* Function to merge b chunks of size chunkSize from the input file to the
specified output file. It internally uses a CHUNK_Iterator and a CHUNK_RecordIterator. */
void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc ){
    // Bway represents the number of chunk that we are merging
    // Create an array of CHUNK_Iterator for b chunks
    CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc , chunkSize);

    // Calculate the number of chunks to create in case the division of the total blocks and the numBlocksInChunck is not an integer

    int totalBlocks = HP_GetIdOfLastBlock(input_FileDesc);
    int numChunks = (totalBlocks + chunkSize - 1) / chunkSize;

    // Initialize chunk +
    CHUNK *chunk = malloc(numChunks * sizeof(CHUNK));
    // Initialize chunk +
    CHUNK_RecordIterator *rec_chunks = malloc(numChunks * sizeof(CHUNK));

    // Initialize an array to keep track of the finished chunks
    bool * chunk_exist = malloc(numChunks * sizeof(bool));
    
    // Initialize record iterators for all the blocks in the chunk
    for (int i = 0; i < numChunks; i++) {
        CHUNK_GetNext(&iterator, &chunk[i]);
        iterator.current ++;
        iterator.lastBlocksID = chunk[i].to_BlockId;
        rec_chunks[i] = CHUNK_CreateRecordIterator( &chunk[i]);
        chunk_exist[i] = true ;
    }

    for(int j=0 ; j<numChunks; j+=bWay)
    {
        printf("b-Way = %d and j = %d \n" , bWay , j);
        if(j+bWay>=numChunks){
            // last merge recalculating bway because the file could have less chunks than the number bway
            bWay= numChunks-j;
        }
        // Merge until they become only one chunk which means that b-way will be one
        while (bWay > 1) {
            Record smallestRecord;
            // Index of the chunk that smallestRecord will be located 
            int smallestIteratorIndex = -1;

            // Find the smallest record across bway iterators
            for (int i = j; i < j + bWay; i++) {

                Record currentRecord;
                if (chunk_exist[i]==false){
                    // All the records of this chunk have been put in the output file
                    continue;
                }
                if (CHUNK_GetNextRecord(&rec_chunks[i], &currentRecord) == 0) {
                    if (smallestIteratorIndex == -1 || shouldSwap(&currentRecord, &smallestRecord)) {
                        smallestRecord = currentRecord;
                        smallestIteratorIndex = i;
                    }
                }
                
            }

            // Write the smallest record to the output file
            // Output file is open so all we need to do is store the records
            HP_InsertEntry(output_FileDesc , smallestRecord);


            // Move to the next record in the chunk that had the smallest record
            if ( rec_chunks[smallestIteratorIndex].cursor + 1 < chunk[smallestIteratorIndex].recordsInChunk){
                rec_chunks[smallestIteratorIndex].cursor ++;
            }
            else{
                //in this case we need to forget about this cursor and know that this chunk is done
                chunk_exist[smallestIteratorIndex] = false;
                bWay--;
            }
            // If the chunk is empty, close its iterator and remove it from the array
        }
    }
    // The first iteration is finished so we call the next one 

}
