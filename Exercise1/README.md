<!-- Readme Top -->
<a id="readme-top"></a>

<p align="center">
  <img src="https://github.com/user-attachments/assets/a0f8e72e-4462-4c02-889d-4bd3887be226" width="150">
</p>

# ${\color{red}Heap \space File \space Implementation \space – \space Assignment 1}$

**Course:** Database Systems Implementation 

**Department:** Informatics & Telecommunications

**Semester:** Winter 2023–2024


 

<!-- TABLE OF CONTENTS -->


<details>
   <summary>Table of Contents</summary>
   <ol>
      <li><a href="Overview">Overview</a></li>
      <li><a href="#Data-Structures">Data Structures</a></li>
      <li><a href="#Implemented-Functions">Implemented Functions</a></li>
      <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
         <li><a href="#exercise1-build">Exercise 1</a></li>
      </ul>
      </li>
      <li><a href="#Execution-Testing">Execution and Testing</a></li>
      <li><a href="#Notes">Notes</a></li>
      <li><a href="#my-pc-stats">My PC Stats</a></li>
   </ol>
</details>


---


<!-- Overview -->
<a id="Overview"></a>

## ${\color{yellow}Overview }$

This project implements a **Heap File management system** in C, built on top of the provided **Block File (BF) layer**.

The goal of the assignment was to understand:

* Block-level memory management
* Record-level storage handling
* How heap-organized files operate internally
* How metadata is maintained and updated on disk

The implementation strictly follows the function prototypes provided in the assignment specification.

<p align="right">(<a href="#readme-top">back to top</a>)</p>


---


<!-- Data Structures -->
<a id="Data-Structures"></a>

## ${\color{yellow}Data \space Structures }$

### `HP_info`

```c
typedef struct {
    int id_last_block;
    int rec_per_block; // maximum 6
} HP_info;
```

The `HP_info` structure stores metadata about the heap file:

* `id_last_block`: The ID of the last data block in the heap file.
* `rec_per_block`: The maximum number of records that fit in each block (maximum 6 records per block).

This structure is stored in the **first block of the file**, which contains only metadata.

---

### `HP_block_info`

```c
typedef struct {
    int records;
    BF_Block *next_block;
} HP_block_info;
```

The `HP_block_info` structure is stored inside each data block and contains:

* `records`: The current number of records stored in the block.
* `next_block`: A pointer to the next data block.

Each block stores:

* Block metadata (`HP_block_info`)
* The actual records


<p align="right">(<a href="#readme-top">back to top</a>)</p>

---


<!-- Implemented Functions -->
<a id="Implemented-Functions"></a>

## ${\color{yellow}Implemented \space Functions }$

---

### `int HP_CreateFile(char *fileName)`

This function:

1. Checks whether a file with the given name already exists.
2. If not, creates a new heap file.
3. Allocates the first block of the file.
4. Stores the file metadata (`HP_info`) inside the first block.

The function uses `BF_OpenFile` in order to allocate the first block that holds the metadata.

Returns:

* `0` on success
* `-1` on failure

---

### `HP_info *HP_OpenFile(char *fileName, int *file_desc)`

This function:

1. Allocates memory for an `HP_info` structure.
2. Opens the file using `BF_OpenFile`.
3. Reads the metadata stored in the first block (using `fread()`).
4. Returns the populated `HP_info` structure.

If any error occurs, the function returns `NULL`.

---

### `int HP_CloseFile(int file_desc, HP_info *hp_info)`

This function:

1. Frees the memory allocated for `hp_info`.
2. Closes the file using `BF_CloseFile`.

Returns:

* `0` on success
* `-1` on failure

---

### `int HP_InsertEntry(int file_desc, HP_info *hp_info, Record record)`

This function inserts a record into the heap file.

There are **three cases**:

---

#### Case 1: No data blocks exist

* Allocate a new block.
* Insert block metadata at the beginning of the block.
* Insert the record after the metadata.
* Mark the block as dirty.
* Unpin the block.

---

#### Case 2: Last block has available space

* Open the last block of the file.
* Skip the block metadata.
* Insert the record at the end of the block.
* Mark the block as dirty.
* Unpin the block.

---

#### Case 3: Last block is full

* Allocate a new block.
* Initialize its metadata.
* Insert the record after the metadata.
* Update the previous block to point to the new block.
* Mark the new block as dirty.
* Unpin the block.

Returns:

* The block ID where the record was inserted
* `-1` on failure

---

### `int HP_GetAllEntries(int file_desc, HP_info *hp_info, int value)`

This function searches and prints all records with:

```
record.id == value
```

Process:

1. Create:

   * A block pointer
   * A block metadata pointer
   * A `void*` pointer
   * A record pointer

2. Start iterating from the **second block** (the first block contains only metadata).

3. For each block:

   * Access its metadata.
   * Iterate through all stored records.
   * Print those with matching `id`.

4. Count how many blocks were read.

Returns:

* The number of blocks read
* `-1` on error


<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- Execution and Testing -->
<a id="Execution-Testing"></a>

## ${\color{yellow}Execution \space and \space Testing }$


The program runs successfully with:

* **600 records** using the provided `hp_main`
* A custom `hp_main1` that:

  * Uses the same logic as the original main
  * Inserts records with **random ID values**

Both test programs execute correctly and validate the heap file functionality.


<p align="right">(<a href="#readme-top">back to top</a>)</p>

---

<!-- GETTING STARTED -->
<a id="getting-started"></a>
## ${\color{yellow}Getting \space Started}$


### 1. Clone this repository
1. Open terminal in the directory that you want to clone project and type
    ```sh
    git clone https://github.com/nataliakrik/hash-tables-chunks-disks.git
   ```


### 2. Go into the repository
2. Open directory
   ```sh
   cd Exercise1
   ```


Instructions on compiling and running the code


<a id="exercise1-build"></a>
### Exercise1
1. Open folder of the exercise you want to test
    ```sh
   cd exercise1
   ```
2. Compile project with makefile
   ```sh
   make bf
   make hp
   make hp1
   ```
3. Input declares polynomial degree
   ```sh
   ./build/bf_main
   ./build/hp_main
   ./build/hp_main1
   ```
   


<p align="right">(<a href="#readme-top">back to top</a>)</p>




<!-- Notes -->
<a id="Notes"></a>

## ${\color{yellow}Notes}$


* The first block of the heap file always stores `HP_info`.
* Metadata is updated when insertions occur and written back to disk.
* `memcpy` is used to copy `Record` data into block memory.
* No primary key constraint is enforced (as specified in the assignment).
* All implementation follows the provided function prototypes exactly.


<p align="right">(<a href="#readme-top">back to top</a>)</p>


---


<!-- PC STATS -->
<a id="my-pc-stats"></a>
## ${\color{yellow}MY \space PC \space STATS}$
|   |   |
|:---:|:---:|
|        ${\color{lightblue} CPU}$       |   AMD Ryzen 7 5700U with Radeon Graphics  |
|  ${\color{lightblue}CPU \space CORES}$ |        8  (16 Logical Processors)         |
|        ${\color{lightblue} OS}$        |         Microsoft Windows 11 Pro          |
| ${\color{lightblue}OS \space VERSION}$ |        10.0.26200 N/A Build 26200         |
|     ${\color{lightblue} COMPILLER}$    | gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0 |



<p align="right">(<a href="#readme-top">back to top</a>)</p>


---

