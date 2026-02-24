#ifndef RECORD_H
#define RECORD_H



typedef enum Record_Attribute {
    ID,
    NAME,
    SURNAME,
    CITY
} Record_Attribute;

typedef struct Record {
    char record[15];
    int id;
    char name[15];
    char surname[20];
    char city[20];
} Record;

/**
 * @brief
 * Creates a random record using rand function to choose name, surname, location.
 * @returns
 * The generated record (type struct Record)
 */
Record randomRecord();

/**
 * @brief
 * Prints record
 * 
 * id:    ,Name:    ,Surname:    ,Location:    
 */
void printRecord(Record record);

#endif
