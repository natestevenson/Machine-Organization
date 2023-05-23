////////////////////////////////////////////////////////////////////////////////
 // Main File:        p3Heap.c
 // This File:        p3Heap.c
 // Other Files:      none
 // Semester:         CS 354 Spring 2023
 // Instructor:       Deppeler
 //
 // Author:           Nathanael Stevenson
 // Email:            nlstevenson@wisc.edu
 // CS Login:         nathanael
 // GG#:              2
 //
 /////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
 //                   fully acknowledge and credit all sources of help,
 //                   other than Instructors and TAs.
 //
 // Persons:          Identify persons by name, relationship to you, and email.
 //                   Describe in detail the the ideas and help they provided.
 //
 // Online sources:   avoid web searches to solve your problems, but if you do
 //                   search, be sure to include Web URLs and description of 
 //                   of any information you find.
///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2023 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission SPRING 2023, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "p3Heap.h"
 
/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           

    int size_status;

    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * Start Heap: 
     *  The blockHeader for the first block of the heap is after skip 4 bytes.
     *  This ensures alignment requirements can be met.
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE NAME or TYPE. 
 * It must point to the first block in the heap and is set by init_heap()
 * i.e., the block at the lowest address.
 */
blockHeader *heap_start = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int alloc_size;

/*
 * Additional global variables may be added as needed below
 * TODO: add global variables needed by your function
 */

 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if size < 1 
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 *   Return if NULL unable to find and allocate block for required size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* balloc(int size) {     
    //TODO: Your code goes in here.

    //make sure size is valid
    if(size < 1){
        return NULL;
    }
    int total = 0;
    //determine whether or not padding is needed
    int modulus = (size+4) % 8;
    int padding = 8 - modulus;
    //calculate final double word block size needed
    if (padding != 8){
        total = size + 4 + padding;
    }else{
        total = size + 4;
    }
    //seach for free memory block to allocate
    blockHeader *current = heap_start;
    while (current->size_status != 1) {
        //check if block is free or allocated
        int a_bit = current->size_status & 1;
        if(!(a_bit)){
            //block is free, check if previous block is free or allocated
            int p_bit = current->size_status & 2;
            int currBlockSize = 0;
            if(p_bit){
                //previous block is allocated, remove p-bit to get block size
                currBlockSize = current->size_status - 2;
                }else{
                //previous block is free
                currBlockSize = current->size_status;
                }
            //check if total is less than or equal to currBlockSize
            if(total <= currBlockSize){
                //current free block is big enough
                if(total == currBlockSize){
                    //current free block is an exact fit
                    //update next blockheader in heap's p-bit as allocated
                    blockHeader *nextBlockHeader = (blockHeader*) ((void*)current + currBlockSize);
                    //make sure the nextBlockHeader is not the END MARK
                    if(nextBlockHeader->size_status != 1){
                        nextBlockHeader->size_status += 2;
                    }
                    current->size_status += 1;//update a-bit to reflect allocation
                    return current+1;//return address of payload start
                }else{
                    //current free block is bigger than needed, check if difference between block size and total memory needed is double word compatible
                    int difference = currBlockSize - total;
                    //if difference is a multiple of 8, splitting is needed
                    if(!(difference % 8)){
                        //remove difference from current free memory block
                        current->size_status -= difference;
                        //set current free memory block a-bit to allocated
                        current->size_status += 1;
                        //record location of next free memory block
                        int nextBlock = currBlockSize - difference;
                        //create new header for unallocated space split from needed space
                        blockHeader *nextBlockHeader = (blockHeader*) ((void*)current + nextBlock);
                        //set size of unallocated space and p-bit to reflect allocated block before it
                        nextBlockHeader->size_status = difference + 2;
                        //create footer for unallocated space
                        blockHeader *nextBlockFooter = (blockHeader*) ((void*)nextBlockHeader + (difference - 4));
                        //set block footer size
                        nextBlockFooter->size_status = difference;
                        return current + 1;//return address of allocated memory payload start
                    }else{
                        //splitting not possible
                        blockHeader *nextBlockHeader = (blockHeader*) ((void*)current + currBlockSize);
                        //make sure nextBlockHeader is not the END MARK
                        if(nextBlockHeader->size_status != 1){
                            nextBlockHeader->size_status += 2;
                        }
                        current->size_status += 1;
                        return current + 1;
                    }    
                }
            }
            //block is free but is not large enough for reqeust, head to the next block
        }
        //block is already allocated, head to the next block
        //remove a-bit from block size
        int blockSize = current->size_status - a_bit;
        //check p-bit status
        int p_bit = current->size_status & 2;
        //if previous block is allocated, remove p-bit from block size
        if(p_bit){
            blockSize -= 2;
        }
        //advance to next block header
        current = (blockHeader*)((void*)current + blockSize);
    }
    return NULL;//unable to find block large enough to fufill request
} 
 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int bfree(void *ptr) {    
    //TODO: Your code goes in here.
    //check that pointer is not null and is a multiple of 8
    if((ptr != NULL) && ( (((unsigned int)ptr) % 8) == 0) ){
        //find heap end
        blockHeader* heap_end = (blockHeader*)((void*)heap_start + (alloc_size-4));
        //check if pointer is outside of heap space
        if( !((unsigned int)ptr < (unsigned int)heap_start || (unsigned int)ptr > (unsigned int)heap_end) ){
            //pointer is within heap space, get size_status variables
            blockHeader *current = (blockHeader*)((void*)ptr - 4);
            int a_bit = current->size_status & 1;
            int p_bit = current->size_status & 2;
            int blockSize = 0;
            //check if block is already freed
            if(a_bit){
                //block is allocated and needs to be freed, check p-bit and block location in heap and get block size
                if(&current != &heap_start){
                    //block is NOT first in heap, check p-bit and get block size
                    if(!(p_bit)){
                    //previous block is free, subtract 1 from size_status
                    blockSize = current->size_status - 1;
                    current->size_status -= 1;//mark current block as free
                    }else{
                    //previous block is allocated, subtract 3 from size_status
                    blockSize = current->size_status - 3;
                    current->size_status -= 1;//mark current block as free
                    }
                    //add blockFooter to newly freed block and update following blocks p-bit if necessary
                    blockHeader *blockFooter = (blockHeader*)((void*)current + (blockSize-4));
                    blockFooter->size_status = blockSize;
                    blockHeader *nextBlock = (blockHeader*)((void*)current + blockSize);
                    //check that nextBlock is not the end of the heap
                    if(nextBlock->size_status != 1){
                    nextBlock->size_status-=2;
                    }
                    return 0;
                }else{
                    //block is allocated and needs to be freed, it IS the first block in the heap so p-bit will always be 1
                    blockSize = current->size_status - 3;
                    current->size_status -= 1;
                    blockHeader *blockFooter = (blockHeader*)((void*)current + (blockSize-4));
                    blockFooter->size_status = blockSize;
                    blockHeader *nextBlock = (blockHeader*)((void*)current + blockSize);
                    nextBlock->size_status-=2;
                    return 0;
                }
            } //block is already freed
        } //pointer is out of the heap space
    }//pointer is null or is not a multiple of 8
    return -1;
} 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for user-called coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    //TODO: Your code goes in here.
    int success = 0;
    blockHeader* current = heap_start;
    while (current->size_status != 1) {
        //check if block is free or allocated
        int a_bit = current->size_status & 1;
        if(!(a_bit)){
            //block is free, check if previous block is free or allocated
            int p_bit = current->size_status & 2;
            int currBlockSize = 0;
            if(p_bit){
                //previous block is allocated, remove p-bit to get block size, check next block header and see if it is free
                currBlockSize = current->size_status - 2;
                blockHeader *nextBlockHeader = (blockHeader*) ((void*)current + currBlockSize);
                //check if next block is free or allocated
                int next_a_bit = nextBlockHeader->size_status & 1;
                int nextBlockSize = 0;
                if(!(next_a_bit)){
                    //next block is free, coalesce with current block
                    nextBlockSize = nextBlockHeader->size_status;
                    int finalBlockSize = currBlockSize + nextBlockSize;
                    current->size_status = finalBlockSize + 2;
                    //set new coalesced block footer
                    blockHeader *currentBlockFooter = ((void*)current + finalBlockSize - 4);
                    currentBlockFooter->size_status = finalBlockSize;
                    success+=1;
                }
                    //else both previous and next block are allocated, no action can be taken, continue
                }else{
                //previous block is free, save current block information and step back to previous block header
                int currBlockSize = current->size_status;
                blockHeader* prevBlockFooter = (void*)current - 4;
                int prevBlockSize = prevBlockFooter->size_status;
                blockHeader* prevBlock = (blockHeader*) ((void*)current - prevBlockSize);
                //acquire prevBlock and p-bit
                int prev_p_bit = prevBlock->size_status & 2;
                //coalesce prevBlock with current
                prevBlock->size_status = currBlockSize + prevBlockSize + prev_p_bit;
                //set current pointer to prevBlock
                current = prevBlock;
                //set newly coalesced current block's footer
                blockHeader* currentFooter = (blockHeader*) ((void*)current + currBlockSize + prevBlockSize - 4);
                currentFooter->size_status = currBlockSize + prevBlockSize;
                success+=1;
                }

            }
        //move the current header forward by 1
        int blockSize = current->size_status;
        int final_a_bit = current->size_status & 1;
        int final_p_bit = current->size_status & 2;
        current = (blockHeader*)((void*)current + (blockSize - final_a_bit - final_p_bit));
        }
     if(success){
        return success;
     }
     return 0;
    }

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int init_heap(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int   pagesize; // page size
    int   padsize;  // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int   fd;

    blockHeader* end_mark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize from O.S. 
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    alloc_size = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    alloc_size -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heap_start = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    end_mark = (blockHeader*)((void*)heap_start + alloc_size);
    end_mark->size_status = 1;

    // Set size in header
    heap_start->size_status = alloc_size;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heap_start->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heap_start + alloc_size - 4);
    footer->size_status = alloc_size;
  
    return 0;
} 
                  
/* 
 * Function can be used for DEBUGGING to help you visualize your heap structure.
 * Traverses heap blocks and prints info about each block found.
 * 
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void disp_heap() {     
 
    int    counter;
    char   status[6];
    char   p_status[6];
    char * t_begin = NULL;
    char * t_end   = NULL;
    int    t_size;

    blockHeader *current = heap_start;
    counter = 1;

    int used_size =  0;
    int free_size =  0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** HEAP: Block List ****************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end p3Heap.c (Spring 2023)                                         


