#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
	BF_ErrorCode code = call; \
	if (code != BF_OK) {         \
		BF_PrintError(code);    \
		return HP_ERROR;        \
	}                         \
}

int HP_CreateFile(char *fileName){

	//Ψαχνει αν υπαρχει αρχειο με αυτο το ονομα
	FILE * file = fopen(fileName, "r");   

	if (file != NULL)
	{ //Αν ισχυει τοτε υπαρχει ηδη αρχειο με αυτο το ονομα
		printf("Failed. File already exists\n");
		fclose(file);
		return -1; 
	}

	//Αφου ελεγχεται η πρωτη περιπτωση τοτε δημιουργει αρχειο με ονομα filename
	file = fopen(fileName, "w");
	if(file == NULL)
	{
		printf("Failed. File was not created\n");
		return -1;
	}

	//Δημιουργια μεταβλητης info τυπου hp_info για την αποθηκευση των μεταδεδομενων του αρχειου
	HP_info info;
	int file_desc;

	//Δημιουργια μεταβλητης μπλοκ και αρχικοποιηση και δεσμευση για την δομη μπλοκ
	BF_Block * block;              
	BF_Block_Init(&block);

	// Δημιουργια δεικτη που θα δειχνει στο μπλοκ για την αποθηκευση μνημη σε αυτο
	void* data;                    

	BF_OpenFile(fileName , &file_desc);         // πaiρνω το file_desc για το αρχειο που δημιουργω απο την συναρτηση BF_openfile
	BF_AllocateBlock(file_desc , block);        // Δημιουργία καινούριου block στο αρχειο με αναγωνριστικο το file_desc
	data = BF_Block_GetData(block);             // Τα περιεχόμενα του block στην ενδιάμεση μνήμη
	info = *(HP_info *)data;                    // Ο δείκτης info δείχνει στην αρχή της περιοχής μνήμης data απο το μπλοκ
	info.id_last_block = 0;
	info.rec_per_block = 6;
	BF_Block_SetDirty(block);                   // κανουμε το μπλοκ dirty και unpin μιας και αλλαξαν τα δεδομενα του
	BF_UnpinBlock(block);

	if (!(fwrite(&info, sizeof(HP_info), 1, file))) //γραφει τα μεταδεδομενα μεσα στο 1ο μπλοκ του αρχειου
	{ 
		fclose(file);
		remove(fileName);                         // Aφαιρεση του αρχειου αν δεν εκτελεστει σωστα 
		printf("Error in writing metadata.\n");
		return -1;                               // Επιστρεφει -1 ως λαθος αφου δεν λειτουργησε σωστα
	}

	fclose(file);
	return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
	
	HP_info* hpInfo = (HP_info*)malloc(sizeof(HP_info)); // Allocate μνημη για hpInfo
	if (hpInfo == NULL) 
	{   //επιστροφη NULL σε περιπτωση error
		printf("Memory allocation error in HP_OpenFile\n");
		return NULL;               
	}
	
	//Ανοιγμα του αρχειου σε binary μορφη 
	FILE* file = fopen(fileName , "rb");
	if(file == NULL)
	{   //επιστροφη NULL σε περιπτωση error
		printf("Error opening the file in HP_OPEN");
		return NULL;        
	}

	//Διαβασμα των μεταδεδομενων του αρχειου μεσω της fread
	if(!(fread(hpInfo, sizeof(HP_info), 1 , file)))
	{   //επιστροφη NULL σε περιπτωση error
		printf("Error in  HP_OPENFILE");
		fclose(file);
		return NULL;      
	}
	//Επιστροφη αναγνωριστικου του αρχειου στη μεταβλητη file_desc
	BF_OpenFile(fileName , file_desc); 
	
	//επιστροφη δεικτη στα μεταδεδομενα του αρχειου εφοσον δεν υπηρξαν errors
	return hpInfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info ){

	// ελευθερωση μνημης για την hp_info
	if (hp_info != NULL)
	{
		printf("HP_info is deleted successfully\n");
		free(hp_info);
	}

	// κλεισιμο αρχειου
	if (BF_CloseFile(file_desc) != BF_OK)
	{
		perror("Error closing the file");
		return -1;
	}
	return 0;
}


int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
	
	int last_block_used = hp_info->id_last_block;

	int blocks_in_file;

		///////////////////ΠΕΡΙΠΤΩΣΗ 1: δημιουργια καινουργιου μπλοκ/////////////////
		if(last_block_used == 0)          //αν ισχυει δεν υπαρχει κανενα αλλο μπλοκ μεσα στο αρχειο εκτος απο το πρωτο
		{  
			BF_Block * block;              //Δημιουργια μεταβλητησ μπλοκ για την αποθηκευση του hp_info
			BF_Block_Init(&block);

			void * data;                    // Δημιουργια δεικτη που θα δειχνει στο μπλοκ για την αποθηκευση μνημη σε αυτο

			BF_AllocateBlock(file_desc , block);        // Δημιουργία καινούριου block στο αρχειο με αναγωνριστικο το file_desc
			data = BF_Block_GetData(block);             // Τα περιεχόμενα του block στην ενδιάμεση μνήμη
			HP_block_info* hpblockinfo = (HP_block_info*)data;
			hpblockinfo->records=1;
			hpblockinfo->next_block = NULL;
			
			int blockinfo_size = sizeof(HP_block_info);  //για να υπολογισουμε την επομενη θεση για το που θα τοποθετηθει η εγγραφη
		
			void *new_data=(char*)data +blockinfo_size;  //δειχνει στην διευθυνση που θα αποθηκευτει η καινουργια εγγραφη
			memcpy(new_data , &record , sizeof(Record)); // αποθηκευει τα δεδομενα το record στο δεικτη data

			BF_Block_SetDirty(block);                    //μετατρεπει το μπλοκ σε dirty επειδη εχουν γινει αλλαγες
			BF_UnpinBlock(block);                        // κσι γινεται και unpin
			return ++(hp_info->id_last_block);           // επιστροφει το id_last_block που δημιουργηθηκε μεσα στο αρχειο 
		}
		///////////////////ΠΕΡΙΠΤΩΣΗ 2 προσθηκη σε ηδη υπαρχων///////////////////////
		else
		{  
			BF_Block* block;
			BF_Block_Init(&block);
			BF_GetBlock(file_desc,hp_info->id_last_block ,block);     //ανοιγμα μπλοκ με id_last_block αρα το τελευτααιο μπλοκ που δημιουργηθηκε

			void* data;
			data = BF_Block_GetData(block);                           // Τα περιεχόμενα του block στην ενδιάμεση μνήμη
			HP_block_info* hpblockinfo = (HP_block_info*)data;
			
			int num_rec = hpblockinfo->records, cur_block_size;

			//Υπολογισμοσ του μεγεθους του block που εχουμε ανοιξει προσθετοντας τον αριθμο των εγγραφων και της hp_block_info
			cur_block_size = (sizeof(Record)*num_rec)+ sizeof(HP_block_info);
			int available_data_size = BF_BLOCK_SIZE - cur_block_size;

			//σε αυτη την περιπτωση εχει χωρο για τουλαχιστον μια ακομη εγγραφη
			if (available_data_size >= sizeof(Record))
			{      
				void *new_data=(char*)data + cur_block_size;    //δειχνει στην διευθυνση που θα αποθηκευτει η καινουργια εγγραφη

				memcpy(new_data , &record , sizeof(Record));    // αποθηκευει τα δεδομενα το record στο δεικτη new_data
				(hpblockinfo->records)++;
				BF_Block_SetDirty(block);                       //μετατρεπει το μπλοκ σε dirty επειδη εχουν γινει αλλαγες
				BF_UnpinBlock(block);                           // και γινεται unpin το μπλοκ

				return (hp_info->id_last_block);                // επιστροφει το id_last_block που δημιουργηθηκε μεσα στο αρχειο 
			}
			////////////ΠΕΡΙΠΤΩΣΗ 3 Δημιουργια καινουργιου επειδη το αλλο ειναι γεματο////////////////
			else
			{
				BF_Block* new_block;
				BF_Block_Init(&new_block);
				BF_AllocateBlock(file_desc , new_block);          // Δημιουργία καινούριου block στο αρχειο με αναγωνριστικο το file_desc

				void *new_data = BF_Block_GetData(new_block);     // Τα περιεχόμενα του block στην ενδιάμεση μνήμη
				hpblockinfo->next_block = new_data;               //δεικτης απο το προηγουμενο μπλοκ δειχνει στο καινουργιο
				
				HP_block_info* new_hpblockinfo = (HP_block_info*)new_data;
				new_hpblockinfo->records=1;                     //καθως θα εισαχθει η πρωτη εγγραφη
				new_hpblockinfo->next_block = NULL;             //δεν δειχνει σε καποιο μπλοκ ακομη οποτε NULL
				
				int blockinfo_size = sizeof(HP_block_info);       //για να υπολογισουμε την επομενη θεση για το που θα τοποθετηθει η εγγραφη
			
				void *new_data1=(char*)new_data +blockinfo_size;  //δειχνει στην διευθυνση που θα αποθηκευτει η καινουργια εγγραφη
				
				memcpy(new_data1 , &record , sizeof(Record));     // αποθηκευει τα δεδομενα το record στο δεικτη data

				BF_Block_SetDirty(new_block);                     //μετατρεπει το μπλοκ σε dirty επειδη εχουν γινει αλλαγες
				BF_UnpinBlock(new_block);                         // γινεται unpin το μπλοκ
				return ++(hp_info->id_last_block);                // επιστροφει το id_last_block που δημιουργηθηκε μεσα στο αρχειο 
			}
		}
	return -1;
}

int HP_GetAllEntries(int file_desc,HP_info* hp_info, int value){    

 //δημιουργια του μπλοκ
	BF_Block * block;
	BF_Block_Init(&block);
	HP_block_info * hpblockinfo;      // δημιουργια δεικτη στα μεταδεδομενα του καθε μπλοκ

	void *data ;
	Record *rec;
	int blocks_in_file= hp_info->id_last_block;
	int block_read=-1;                // ο αριθμος των μπλοκ που θα διαβαστηκαν για να επιστραφει στο τελος της συναρτησης
	
	//εξωτερικη επαναληψη ολων των μπλοκ του αρχειου 
	for (int j = 1; j <= blocks_in_file; j++){
		
		BF_GetBlock(file_desc, j , block);
	
		data = BF_Block_GetData(block);
		
		//αποθηκευση διευθυνσης των πληροφοριων του μπλοκ
		hpblockinfo = (HP_block_info*)data;
		data=(char*)data + sizeof(HP_block_info);     //offset για να παμε στην διευθυνση που αποθηκευονται οι εγγραφες του μπλοκ
		// εσωτερικη επαναληψη για ολες τις εγγραφες του καθε μπλοκ
		for (int i = 0; i < hpblockinfo->records; i++)
		{
			rec=(Record*)data;
			if(rec->id == value)                        //αν η εγγραφη εχει το σωστο id, εκτυπωσε τη
			{
				printf("ID : %d\tNAME : %-10s\tSURNAME : %-10s\tCITY : %-10s\n", rec->id, rec->name, rec->surname, rec->city);
				//η μεταβλητη block_read κραταει ποσα μπλοκ εχουν διαβαστει μεχρι στιγμης αρα και ποσα μπλοκ πρεπει να διαβαστουν για την ευρεση των εγγραφων με id=value
				block_read=j;     
			}
			data=(char*)data + sizeof(Record);          //offset για να παμε στην επομενη εγγραφη 
		}
		if (hpblockinfo->next_block== NULL)           //ελεγχει αν υπαρχει επομενο μπλοκ και συνεχιζει η τερματιζει αναλογα
		{
			if(block_read==-1)                          //αν το block_read = -1 και δεν υπαρχει επομενο μπλοκ σημαινει οτι δεν βρεθηκε η εγγραφη
				printf("\nThe record was not found\n");
			BF_UnpinBlock(block);   
			return block_read ;                       //επιστροφη του τελευταιου μπλοκ που περιειχε εγγραφη με id=value
		}
		BF_UnpinBlock(block);                       //κανει unpin το block διοτι εχει γινει pin απο την bf_getblock
	}    
	return -1;
}

