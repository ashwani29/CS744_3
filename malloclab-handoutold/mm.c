/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "mm.h"
#include "memlib.h"
#include "config.h" // ***** To be removed *******

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "XXXX",
    /* First member's full name */
    "Amit Jaiswal",
    /* First member's email address */
    "amitjaiswal@cse.iitb.ac.in",
    /* Second member's full name (leave blank if none) */
    "Ashwani Kumar Yadav",
    /* Second member's email address (leave blank if none) */
    "ashwaniydv@cse.iitb.ac.in"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


static u_int64_t *mm_start_brk;
static u_int64_t *mm_max_addr;

char binary_buffer[21];

#define HEADER_FOOTER_SIZE 8
#define INT64_T_SIZE 8

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
	/*printf("\n********heap start address = %p*********\n", mem_sbrk(0));
	printf("********heap end address = %p*********\n", mem_sbrk(0) + MAX_HEAP);
    printf("****** page_size = %d *****\n", mem_pagesize());
    printf("****** heap_size = %d *****\n", mem_heapsize());
    printf("****** heap_lo = %p *****\n", mem_heap_lo());
    printf("****** heap_hi = %p *****\n", mem_heap_hi());
    int i = 44;
    printf("****** align(%d) = %d *****\n", i, ALIGN(i));
    printf("****** SIZE_T_SIZE = %d *****\n", SIZE_T_SIZE);*/

    //printf("block_size:%d\n", mm_max_addr - mm_start_brk);
    //printf("sizeof : %d \n", sizeof(int64_t));

    //mm_start_brk[0] = 254;
    //printf("\n********heap start address = %p*********\n", mem_sbrk(0));
    //printf("&mm_start_brk[0] = %p\n", &mm_start_brk[0]);
    //printf("&mm_start_brk[1] = %p\n", &mm_start_brk[1]);

    //printf("total_memory = %d\n", mm_max_addr - mm_start_brk);

    u_int64_t block_size;
    mm_start_brk = mem_sbrk(0);
    mm_max_addr = mem_sbrk(0) + MAX_HEAP;

    block_size = (mm_max_addr - mm_start_brk) * INT64_T_SIZE;
    mm_start_brk[0] = ALIGN(block_size);
    *(mm_max_addr - 1) = ALIGN(block_size);

    //printf("start_brk[0] = %lld max_addr[-1] = %lld \n", mm_start_brk[0] | (int64_t)1, *(mm_max_addr - 1));

    //u_int64_t x = 7;
    //printf("comp(7): %llu\n", ~x);
    //printf("size = %d\n", sizeof(x));

	return 0;
}


void *find_fit(size_t size) {

    if (((mm_start_brk + size) > mm_max_addr)) {
    errno = ENOMEM;
    fprintf(stderr, "****ERROR: mem_sbrk failed. Ran out of memory...\n*******");
    return (void *)-1;
    }
    u_int64_t *header = mm_start_brk;
    //printf("mm_start_brk = %p    mm_start_brk[0] = %llu\n", mm_start_brk, mm_start_brk[0]);
    while(header < mm_max_addr) {
        if(!(*header & (u_int64_t)2) && header[0] >= size)
            return header;
        header = header + (header[0] & ~((u_int64_t)7))/INT64_T_SIZE;
        //printf("header: %p header[0]: %llu in findfit\n", header, header[0]);
        //sleep(5);
    }
    return NULL;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    /*
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
    */

    ///*

    //printf("\n*************************************************************\n");
    //printf("mm_start_brk = %p mm_start_brk[0] = %llu\n", mm_start_brk, mm_start_brk[0]);
    //printf("mm_max_addr = %p\n", mm_max_addr);
    //printf("request size = %d\n", size);

    if ( (size < 0) || ((mm_start_brk + size) > mm_max_addr)) {
    errno = ENOMEM;
    fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
    return (void *)-1;
    }
    size_t alloc_blk_size = ALIGN(size + 2 * HEADER_FOOTER_SIZE);
    u_int64_t original_blk_size;
    u_int64_t remaining_blk_size;
    u_int64_t *header = find_fit(alloc_blk_size);
    //printf("Returned from find_fit with address %p\n", header);

    int split = 0;
    if(header) {
        mem_sbrk(alloc_blk_size);
        original_blk_size = header[0];
        remaining_blk_size = (original_blk_size & ~(u_int64_t)7) - alloc_blk_size;

        //printf("original_blk_size = %llu remaining_blk_size = %llu\n", (original_blk_size & ~(u_int64_t)7), remaining_blk_size);

        if(remaining_blk_size > 16)
                split = 1;
        if(split) header[0] = alloc_blk_size;

        //updating allocated block header
        header[0] = header[0] | (u_int64_t)2; // updating C
        if((original_blk_size & (u_int64_t)4) == 4) { // updating L
                header[0] = header[0] | (u_int64_t)4;
        }

        if(!split) { // updating R
            if(original_blk_size & (u_int64_t)1){
                header[0] = header[0] | (u_int64_t)1;

		}
        }

        //printf("allocated  block header_addr:%p footer_addr:%p \n", header, header - 1 + (header[0] & ~(u_int64_t)7)/INT64_T_SIZE);

        //updating allocated block footer
        *(header + (header[0] & ~(u_int64_t)7)/INT64_T_SIZE - 1) = header[0];
        //printf("allocated  block header_val:%lld footer_val:%lld \n", header[0], *(header + alloc_blk_size/INT64_T_SIZE - 1));


        if(header - 1 >= mm_start_brk) { // check left block present
            //updating left block of allocated block
            //printf("I'm in left block\n");
            *(header - 1) = *(header - 1) | (u_int64_t)1; // uodating footer
            (header - (*(header - 1) & ~(u_int64_t)7)/INT64_T_SIZE)[0] = *(header - 1); // updating header
        }

        if(split) { // check if split occurs
            // updating remaining block
            //printf("I'm in remaining block\n");
            *(header + alloc_blk_size/INT64_T_SIZE) = remaining_blk_size; // updating header
            *(header + alloc_blk_size/INT64_T_SIZE) = *(header + alloc_blk_size/INT64_T_SIZE) | (u_int64_t)4; // updating  L
            if((header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE) < mm_max_addr) { // check if right block present
                if(((header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE)[0] & (u_int64_t)2) == 2) { // updating R
                        *(header + alloc_blk_size/INT64_T_SIZE) = *(header + alloc_blk_size/INT64_T_SIZE) | (u_int64_t)1;
                }
            }

            //printf("hdr: %p alc_blk_size: %llu rem_blk_hdr: %p value: %llu\n", header, alloc_blk_size, header + alloc_blk_size/INT64_T_SIZE, *(header + alloc_blk_size/INT64_T_SIZE));

            *(header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE - 1) = *(header + alloc_blk_size/INT64_T_SIZE); //updating footer
        }

        else{
            if((header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE) < mm_max_addr) {
            // updating right block of remaining block
                if((*(header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE - 1) & (u_int64_t)2) == 2) { //updating header
                        (header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE)[0] = (header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE)[0] | (u_int64_t)4;// updating L
                }
            // updating footer
                (header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE + ((header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE)[0] & ~(u_int64_t)7) - 1)[0]  = (header + (original_blk_size & ~(u_int64_t)7)/INT64_T_SIZE)[0]; //updating footer
            }
        }
        //printf("malloc_return_addr = %p headr[0] = %llu\n", (header+1), header[0]);
        //printf("*************************************************************\n");
        return (header+1);

    }
    else {
        return NULL;
    }


    //*/
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

    u_int64_t *blk_header = ptr, *l_blk_header, *r_blk_header, least3_sig_bits_l, least3_sig_bits_c, least3_sig_bits_r;
    int free_operation_done = 0;

    blk_header = blk_header - 1; // to get the header address
    least3_sig_bits_c = (blk_header[0] & (u_int64_t)7);

    if(!(blk_header[0] & (u_int64_t)4)) { // check if left free
        l_blk_header = blk_header - ((blk_header - 1)[0] & ~(u_int64_t)7)/INT64_T_SIZE;

        if(l_blk_header >= mm_start_brk){
        	least3_sig_bits_l = (l_blk_header[0] & (u_int64_t)7);

        	// update header of the left block
        	l_blk_header[0] = (l_blk_header[0] & ~(u_int64_t)7) + (blk_header[0] & ~(u_int64_t)7);  // update size of block
        	if(least3_sig_bits_l & (u_int64_t)4)    // update l
            		l_blk_header[0] = l_blk_header[0] | (u_int64_t)4;
        	if(least3_sig_bits_c & (u_int64_t)1)    // update r
            		l_blk_header[0] = l_blk_header[0] | (u_int64_t)1;

        	// update footer of the current block
        	(blk_header + (blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE - 1)[0] = l_blk_header[0];
            blk_header = l_blk_header;
        }
        // update right block
        r_blk_header = (blk_header + (blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE);
        if(r_blk_header < mm_max_addr){
        		r_blk_header[0] = r_blk_header[0] & ~(u_int64_t)4; // update l to 0
        		(r_blk_header + (r_blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE - 1)[0] = r_blk_header[0]; // update footer
        }
        free_operation_done = 1;
	//printf("Left block is free --block header is at %p\n",blk_header);
    }

    if(!(blk_header[0] & (u_int64_t)1)) { // check if right free
        r_blk_header = (blk_header + (blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE);
        least3_sig_bits_r = (r_blk_header[0] & (u_int64_t)7);

        if(r_blk_header < mm_max_addr){
        	// update header of the current block
        	blk_header[0] = (blk_header[0] & ~(u_int64_t)7) + (r_blk_header[0] & ~(u_int64_t)7);  // update size of block
        	if(least3_sig_bits_c & (u_int64_t)4)    // update l
                    blk_header[0] = blk_header[0] | (u_int64_t)4;
        	if(least3_sig_bits_r & (u_int64_t)1)    // update r
            		blk_header[0] = blk_header[0] | (u_int64_t)1;

        	// update footer of the right block
        	(r_blk_header + (r_blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE - 1)[0] = blk_header[0];

        }
        // update left block
        l_blk_header = (blk_header - ((blk_header-1)[0] & ~(u_int64_t)7)/INT64_T_SIZE);
        if(l_blk_header >= mm_start_brk){
        		l_blk_header[0] = l_blk_header[0] & ~(u_int64_t)1; // update r to 0
        		(blk_header - 1)[0] = l_blk_header[0]; // update footer
        }
        free_operation_done = 1;
        //printf("right block is free --block header is at %p\n",blk_header);
    }

    if((blk_header[0] & (u_int64_t)1) && (blk_header[0] & (u_int64_t)4) && !free_operation_done) { // check if left not free and right not free
        // update current block
        blk_header[0] = blk_header[0] & ~(u_int64_t)2;  // update c in header
        (blk_header + (blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE - 1)[0] = blk_header[0]; // update footer

        // update left block
        l_blk_header = (blk_header - ((blk_header-1)[0] & ~(u_int64_t)7)/INT64_T_SIZE);
        if(l_blk_header >= mm_start_brk){
                l_blk_header[0] = l_blk_header[0] & ~(u_int64_t)1; // update r to 0
                (blk_header - 1)[0] = l_blk_header[0];
         }
        // update right block
        r_blk_header = (blk_header + (blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE);
        if(r_blk_header < mm_max_addr){
                r_blk_header[0] = r_blk_header[0] & ~(u_int64_t)4;  // update l to 0
                (r_blk_header + (r_blk_header[0] & ~(u_int64_t)7)/INT64_T_SIZE - 1)[0] = r_blk_header[0];
        }
        //printf("Left and Right were not free --block header is at %p\n",blk_header);
    }

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














