ΑΡΙΘΜΟΣ ΟΜΑΔΑΣ: 72
ΜΕΛΗ: ΝΑΤΑΛΙΑ ΚΡΙΚΕΛΛΗ(1115202000104) , ΤΙΤΟΣ ΜΠΑΡΤΣΩΤΑΣ (1115202100268) 

ΣΥΝΟΨΗ ΛΟΓΙΚΗΣ ΣΥΝΑΡΤΗΣΕΩΝ:

Για την συνάρτηση shouldSwap() , χρησιμοποιήσαμε την συνάρτηση strcmp() για να γίνει
η σύγκριση μεταξύ των ονομάτων των εγγραφών και απλά επιστρέφεται αποτέλεσμα σχετικά με το αν χρειάζεται να γίνει η 
εναλλαγή. 

Για την συνάρτηση sort_FileInChunks() , η δημιουργία των συρμών γίνεται 'εικονικά' , δηλαδή μέσω ενός iterator 
δημιουργούνται τα chunks για το καθένα από τα οποία κρατάμε τα id του πρώτου και του τελευταίου μπλοκ που περιέχουν.
Το κάθε chunk περιέχει ως πρώτο μπλοκ το επόμενο μπλοκ από το τελευταίο του προηγούμενου chunk κ.ο.κ. 
Όταν φτάσουμε στο τελευταίο chunk που ενδεχομένως περιέχει λιγότερες εγγραφές από τα προηγούμενα , φροντίζεται ο αριθμός
 των εγγραφών που περιέχει να είναι ο σωστός. Για κάθε chunk που δημιουργείται , εκτελείται και η συνάρτηση sort_Chunk().

Για την συνάρτηση sort_Chunk() , η κύρια λογική είναι η εξής: Μέσω του bubble sort , παίρνουμε τη πρώτη εγγραφή του πρώτου 
μπλοκ μέσα στο chunk και το συγκρίνουμε με κάθε άλλη εγγραφή μέσα στο chunk και αν χρειάζεται γίνονται εναλλαγές ώστε να
 πάει στη σωστή θέση. Επειδή χρησιμοποιείται το bubble sort οι εγγραφές που είναι ταξινομιμένες πάνε στο τέλος της σειράς ,
 άρα σε κάθε επανάληψη φροντίζουμε να μην πειραχτούν ξανά οι ήδη ταξινομιμένες εγγραφές. Συνεχίζεται αυτή η διαδικασία 
μέχρι όλες οι εγγραφές του chunk να βρίσκονται στη σωστή θέση.

Για την συνάρτηση merge η κυρια λογικη ειναι η εξης :
Στην τελικη συναρτηση δημιουργησαμε εναν iterator για να διαχειριζομαστε σωστα τα chunks ,
εναν πινακα απο record iterators για να διαχειριζομαστε τις εγγραφες του καθε chunk , 
εναν πινακα απο chunks για να εχουμε ευκολη προσβαση σε καθε chunk του αρχειου
εναν λογικο πινακα για να γνωριζουμε αν το chunk εχει ταξινομηθει ή αν εχει ακομα εγγραφες που 
δεν εχουν ταξινομηθει.
Η πρωτη επαναληψη ειναι ποσες συγχωνεύσεις πρεπει να γινουν μεσα στο αρχειο.
Αφου το bway ειναι μεγαλυτερο απο 1 σημαινει οτι υπαρχουν ακομη chunks που θελουν συγχώνευση.
Για καθε ενα chunk απο αυτα συγκρινουμε την εγγραφη στην οποια δειχνει ο κέρσορας (record iterators)
του καθε chunk και κραταμε την μικροτερη.
Αφου τελειωσουμε με τις επαναληψεις τοποθετούμε την εγγραφη μεσα στο αρχειο εξοδου και αναβαθμισζουμε 
τα χαρακτηριστικά του κέρσορα του συγκεκριμένου chunk και ελεγχουμε αν αυτο το chunk περιεχει αλλες 
εγγραφες ή αν τελειωσε.

Η συναρτηση merge δεν δουλευει σωστα παρουσιαζει error:Floating point exception και φαινεται να σταματαει να τρεχει 
εκει που καλει την insert για να τοποθετησει την εγγραφη στο αρχειο εξοδου. Δυστυχως δεν μπορεσαμε να καταλβουμε γιατι
παρουσιαζει τετοιο error.


------------------------------------------------------------------------------------------------------------------------
ΣΥΝΑΡΤΗΣΕΙΣ ΚΑΙ ΣΥΓΚΡΙΣΗ ΜΕ ΤΟ CHATGPT:

Πρωτη Συναρτηση:
bool shouldSwap(Record* rec1,Record* rec2)
Για αυτην την συναρτηση το chat μου εδωσε αυτην την απάντηση:

bool shouldSwap(Record* rec1, Record* rec2) {
    // Compare names
    int nameComparison = strcmp(rec1->name, rec2->name);

    if (nameComparison == 0) {
        // If names are equal, compare surnames
        int surnameComparison = strcmp(rec1->surname, rec2->surname);
        return surnameComparison > 0;  // Swap if rec1's surname comes after rec2's surname
    }

    return nameComparison > 0;  // Swap if rec1's name comes after rec2's name
}

Η συναρτηση ηταν σωστη και η μονη αλλαγη που εγινε ειναι οτι χρησιμοποιησαμε μια μεταβλητη.
Ετσι κανουμε return μια φορα στο τελος της συναρτησης.
Το link της συζήτησης βρίσκεται απο κατω:
https://chat.openai.com/share/dd838ef9-a295-4eda-8ca0-9e3f692dfc88


Δευτερη Συναρτηση:
Για την συναρτηση sort_FileInChunks η απαντηση που πηραμε απο το chat ηταν σχετικα ελλιπης
καθως δεν ειχε προσβαση σε ολες τις βιβλιοθηκες και τις συναρτησεις που γνωριζουμε εμεις.
Για αυτο τον λογο χρειάστηκαν να γινουν αρκετες αλλαγες στην τελικη συναρτηση αλλα η αρχικη 
λογικη ειναι ιδια. Οι αλλαγες που εγινα αφορουσαν κυρίως την αρχικοποιηση των χαρακτηριστικών
ενος chunnk. Επισης το αρχικο κομματι για την δημιουργια και το ανοιγμα τοου αρχειου ηταν 
περιττό .


// Function to sort blocks in a file using chunks
void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    // Open the file
    FILE* file = fdopen(file_desc, "r+");

    // Get the total number of blocks in the file
    fseek(file, 0, SEEK_END);
    int totalBlocks = ftell(file) / sizeof(Record);
    rewind(file);

    // Calculate the number of chunks
    int numChunks = (totalBlocks + numBlocksInChunk - 1) / numBlocksInChunk;

    // Iterate over chunks
    for (int chunkIndex = 0; chunkIndex < numChunks; ++chunkIndex) {
        // Create a CHUNK for the current chunk
        CHUNK chunk;
        chunk.file_desc = file_desc;
        chunk.from_BlockId = chunkIndex * numBlocksInChunk;
        chunk.to_BlockId = (chunkIndex + 1) * numBlocksInChunk - 1;
        chunk.recordsInChunk = numBlocksInChunk * 7;  // Assuming each block contains 7 records
        chunk.blocksInChunk = numBlocksInChunk;

        // Handle the last chunk, which may have fewer blocks
        if (chunk.to_BlockId >= totalBlocks) {
            chunk.to_BlockId = totalBlocks - 1;
            chunk.recordsInChunk = (chunk.to_BlockId - chunk.from_BlockId + 1) * 7;
            chunk.blocksInChunk = chunk.to_BlockId - chunk.from_BlockId + 1;
        }

        // Sort the chunk
        sort_Chunk(&chunk);
    }

    // Close the file
    fclose(file);
}

Το link της συζήτησης ειναι αυτο:
https://chat.openai.com/share/8885952e-b149-4cfb-b3f7-b06744298bea


Τριτη Συναρτηση:
Για την συναρτηση sort_Chunk ο κώδικας που πηραμε απο το chat χρειάστηκε αρκετες αλλαγές
καθως η λογικη της ταξινόμησης φουσκαλιδα ειναι σωστη δεν καταφερε να προσαρμοσει τα δεδομένα
της ασκησεις πανω σε αυτην.
Οποτε και σε αυτην την συναρτηση εγιναν αρκετες τροποποιήσεις κυριως πανω στο κομματι συγκρισης
μιας εγγραφης ενος block με μια εγγραφη που βρισκοταν σε αλλο block μεσα στο ιδιο chunk, για να 
προσδιοριστει η θεση της έγγραφης. Πιο κατω ειναι ο κωδικας απο το chat gpt:

void sort_Chunk(CHUNK* chunk)

/* Sorts records within a CHUNK in ascending order based on the name and
surname of each person. */
void sort_Chunk(CHUNK* chunk) {
    int i, j;

    for (i = 0; i < chunk->recordsInChunk - 1; i++) {
        for (j = 0; j < chunk->recordsInChunk - i - 1; j++) {
            Record* currentRecord = (Record*)(BF_Block_GetData(chunk->block) + j * sizeof(Record));
            Record* nextRecord = (Record*)(BF_Block_GetData(chunk->block) + (j + 1) * sizeof(Record));

            if (shouldSwap(currentRecord, nextRecord)) {
                // Swap the records
                Record temp;
                memcpy(&temp, currentRecord, sizeof(Record));
                memcpy(currentRecord, nextRecord, sizeof(Record));
                memcpy(nextRecord, &temp, sizeof(Record));
            }
        }
    }
}


Το link της συζήτησης ειναι αυτο:
https://chat.openai.com/share/b739963e-36fc-4588-9ef8-4d2a153dde4d



Τεταρτη Συναρτηση

Για την συναρτηση merge η λογικη του κωδικα που πηραμε απο το chat ειναι αρκετα σωστη.
Ενα λαθος ειναι οτι σταματει της συγχωνευση chunk στα πρωτα bway chunk  δηλαδη δεν 
κανει bway merge σε ολα τα chunks του αρχειου. Επισης αντι για recordIterator δημιουργησε 
chunk iterator.
Παρακατω ειναι η υλοποιηση του chatgpt:


/* Function to merge b chunks of size chunkSize from the input file to the
specified output file. It internally uses a CHUNK_Iterator and a CHUNK_RecordIterator. */
void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Create an array of CHUNK_Iterator for b chunks
    CHUNK_Iterator* iterators = malloc(bWay * sizeof(CHUNK_Iterator));
    if (!iterators) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize iterators for the first block of each of the b chunks
    for (int i = 0; i < bWay; i++) {
        iterators[i] = CHUNK_CreateIterator(input_FileDesc, chunkSize);
        CHUNK_GetNext(&iterators[i], NULL); // Move to the first block
    }

    // Open output file for writing
    FILE *outputFile = fopen("output.txt", "w");
    if (!outputFile) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Merge until only one chunk remains
    while (bWay > 1) {
        Record smallestRecord;
        int smallestIteratorIndex = -1;

        // Find the smallest record across b iterators
        for (int i = 0; i < bWay; i++) {
            Record currentRecord;
            if (CHUNK_GetNextRecord(&iterators[i], &currentRecord) == 0) {
                if (smallestIteratorIndex == -1 || shouldSwap(&currentRecord, &smallestRecord)) {
                    smallestRecord = currentRecord;
                    smallestIteratorIndex = i;
                }
            }
        }

        // Write the smallest record to the output file
        fwrite(&smallestRecord, sizeof(Record), 1, outputFile);

        // Move to the next record in the chunk that had the smallest record
        if (CHUNK_GetNextRecord(&iterators[smallestIteratorIndex], NULL) != 0) {
            // If the chunk is empty, close its iterator and remove it from the array
            CHUNK_Iterator temp = iterators[smallestIteratorIndex];
            for (int j = smallestIteratorIndex; j < bWay - 1; j++) {
                iterators[j] = iterators[j + 1];
            }
            bWay--;

            // Free the resources of the closed iterator
            fclose(temp.file_desc);
        }
    }

    // Close the output file
    fclose(outputFile);

    // Free the memory allocated for iterators
    free(iterators);
}
Το link της συζήτησης ειναι αυτο:
https://chat.openai.com/share/99f446a9-cc64-4356-b6f1-e2e05e7dd792


