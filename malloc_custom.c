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
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//typedef uintptr_t uint;
typedef int bool;

#define true 1
#define false 0
/*Yeah, I need to take this seperatley because
 of the behaviour of malloc which may not give you 
adjacent locations after successive call to it.
Therefore, we need to explicitly define prev and adjacent
blocks to block */
struct adjacent_info{
    uintptr_t prev_adj;
    uintptr_t next_adj;
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
    uintptr_t next;
    uintptr_t prev;
    /*
      size of this block. This size will be in number of blocks.
      not in bytes.Excluding this header.
    */
    uintptr_t size;
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


static header_t* pdummy=0;
static header_t* ptail=0;


/*Ask some predefine number of blocks from mallocs and give
  us an initialised free block already added in the free list*/
header_t* get_more_memory(){
    /*always get INIT_SIZE from a seperate allocator. */
    uintptr_t size = INIT_SIZE;
    void *p = malloc(size);
    header_t *temp=NULL;
    if(!p){
	return;
    }
    /*if pdummy is null init it.*/
    if(!pdummy){
	pdummy = (header_t*)p;
	/*Adjust p and size*/
	p+=sizeof(*pdummy);
	size-=sizeof(*pdummy);
	pdummy->next=0;
	pdummy->size=0;
	pdummy->adj.prev_adj=0;
	pdummy->adj.next_adj=0;
    }
    /*Now add this*/
    temp = (header_t*)p;
    temp->adj.next_adj = temp->adj.prev_adj = 0;
    temp->size=size-sizeof(header_t);
    if(!pdummy->next){
	pdummy->next = (uintptr_t)temp;
    }
    /*New block will always be added at tail.*/
    if(!ptail){
	ptail=temp;
    }else{
	ptail->next=(uintptr_t)temp;
	ptail = temp;
    }
    ptail->next = (uintptr_t)pdummy;
    return ptail;
}

/*split the pblock in two blocks one of new_size if splittable else return NULL*/
static header_t* split_block(header_t* pblock,size_t new_size){
    uintptr_t size=0;
    header_t* pres=NULL;
    header_t* temp;
    if(pblock==NULL || size==0 )
	return NULL;
    size = pblock->size+MIN_BLOCK-(new_size+MIN_BLOCK);
    if(size<=0){
	/*Not splittable.*/
	return NULL;
    }
    /*update the size of old block.*/
    pblock->size -= (new_size+MIN_BLOCK);
    pres = pblock+MIN_BLOCK+(pblock->size)*MIN_BLOCK;
    pres->size = new_size;
    /*assume the new block as fresh. so don't
      add it free list and don't change the next field
      of parent block.*/
    pres->next = 0;
    /*now as both block are free update their adj link.*/
    pres->adj.prev_adj=(uintptr_t)pblock;
    pres->adj.next_adj=(uintptr_t)pblock->adj.next_adj;
    if(pblock->adj.next_adj!=0){
	temp = ((header_t*)(pblock->adj.next_adj));
	temp->adj.prev_adj = ((uintptr_t)pres);
    }

    pblock->adj.next_adj = (uintptr_t)pres;
    return pres;
}
/*
 * p must be in free list
 */
static header_t* remove_frm_free_list(header_t* p){
    header_t *ppnext=0,*pprev=0;
    ppnext=(header_t*)(p->next);
    pprev = (header_t*)(p->prev);
    if(ppnext==NULL || pprev==NULL){
	printf("shouldn't be null");
	return NULL;
    }
    pprev->next = (uintptr_t)(ppnext);
    ppnext->prev=(uintptr_t)(pprev);
    p->next=0;
    p->prev=0;
    return p;

}

static void add_to_free_list(header_t* p){
    header_t* pprev=(header_t*)(ptail->prev); 
    if(!IS_FREE(p)){
	printf("Invalid node\n");
    }
    p->next = ptail->next;
    p->prev = (uintptr_t)ptail;
    ptail->next =(uintptr_t) p;
    ptail = p;
    return;
}
/*
 *Merge p to it's adjacent block if possible and
 *to the free list.Aftrer fucntion returns we can be
 *sure that p is added to free list. 
 */
static bool merge_block(header_t* p){
    header_t *ppnext=0,*pprev=0;
    header_t *pppnext;
    bool bresult = false;
    if(!p){
	return false;
    }
    
    /*check if next adjacent is mergable*/
    if(p->adj.next_adj && IS_FREE(p->adj.next_adj)){
	ppnext = remove_frm_free_list((header_t*)p->adj.next_adj);
    }
    
    /*now see if prev can also be merged or */
    if(p->adj.prev_adj && IS_FREE(p->adj.prev_adj)){
	pprev = remove_frm_free_list((header_t*)p->adj.prev_adj);
    }

    if(pprev || ppnext){
	bresult = true;
    }

    /*1st merge to next block*/
    if(ppnext){
	pppnext = (header_t*)((header_t*)ppnext->adj.next_adj);
	if(pppnext){
	    pppnext->adj.prev_adj = ((uintptr_t)p);
	}
	p->adj.next_adj = (uintptr_t)(pppnext);
	p->size+=(ppnext->size+MIN_BLOCK);
    }
    
    if(pprev){
	pppnext = (header_t*)(p->adj.next_adj);
	if(pppnext){
	    pppnext->adj.prev_adj = ((uintptr_t)pprev);
	}
	pprev->adj.next_adj = (uintptr_t)(pppnext);
	pprev->size+=(ppnext->size+MIN_BLOCK);
	p=pprev;
    }

    /*Now add p to tail*/
    add_to_free_list(p);
    return bresult;
}


static void malloc_init(){
    if(pdummy){
	/*already initalised so return*/
	return;
    }

}
