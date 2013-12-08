/*
  This is a bit of practise stuff in C. The folowing module
  is a bit of replica of malloc implementaion from the great book
  by Kernighan. It implementas a free list based custom allocator
  in user space. Each memory request is round up to minimum number 
  of blocks the can be allocated. Whenever a request is arrived we 
  will look for a block which has size exactly equal or greater then
  the request 
*/
#include <stddef.h>
typedef unsigned int uint;
/*Yeah, I need to take this seperatley because
 of the behaviour of malloc which may not give you 
adjacent locations after successive call to it.
Therefore, we need to explicitly define prev and adjacent
blocks to block */
struct adjacent_info{
    uint prev_adj;
    uint next_adj
};
/*declare a struct header*/
struct blk_header{
    /*
      Address of the next block in the free list.
      The next block may not necessary be the adjacent block.
      Advantage:Will save us time in finding a next
      free block.
      Disadvantage:These fields are incresing the size of min block.
      And prone to incresing the internal fragmentation.
    */
    uint next;
    /*
      size of this block. This size will be in number of blocks.
      not in bytes.Excluding this header.
    */
    uint size;
    /*
      Define adjacent information for a block.Make the merge and 
      split faster.
     */

    struct adjacent_info adj;
};
typedef struct blk_header header_t;
/*Macros for removing confusion while calculation.*/
#define MIN_BLOCK                   sizeof(header_t)
#define ROUND_NO_BLOCKS(bytes)      (bytes+MIN_BLOCK-1)/(MIN_BLOCK)
#define NEXT_ADJACENT_BLOCK(p)      (((header_t*)p)->adj.next_adj)
#define PREV_ADJACENT_BLOCK(p)      (((header_t*)p)->adj.prev_adj)
/*Therefor our list is a circular list*/
#define IS_FREE(p)                  ((((header_t*)p)->next)!=0)
#define INIT_SIZE                   ((1<<8)*MIN_BLOCK)
static uint head=0;


static void malloc_init(){
    if(head){
	/*already initalised so return*/
	return;
    }
    
}
