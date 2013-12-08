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
/*declare a struct header*/
struct blk_header{
  /*
    address of the next free block.
    Advantage:Will save us time in finding a next
    free block.
    Disadvantage:These fields are incresing the size of min blocks.
    And prone to incresing the internal fragmentation.
  */
  uint next;
  /*
    size of this block. This size will be in number of blocks.
    not in bytes
  */
  uint size;
  /*
    prev adjacent block. 
  Merging of the block will be 
  faster.
  */
  
};
