#ifndef MAPALLOC_H
#define MAPALLOC_H

#include <stddef.h>		/* for the definition of size_t */

void *map_malloc(size_t n);
void *map_calloc(size_t nelem, size_t elsize);
void *map_realloc(void *ptr, size_t n);
void map_free(void *ptr);

#endif
