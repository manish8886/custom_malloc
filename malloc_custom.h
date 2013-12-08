#include <stdio.h>
#include <stddef.h>
/*allocate a continuous block of size 'size'.*/
void* malloc_custom(size_t size);
/*free pointer 'p' previously all3ocated by malloc_custom. */
void malloc_free(void* p);
