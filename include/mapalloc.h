#ifndef MAPALLOC_H
#define MAPALLOC_H

#include <stddef.h>		/* for the definition of size_t */

void *MA_malloc(size_t n);
void *MA_calloc(size_t nelem, size_t elsize);
void *MA_realloc(void *ptr, size_t n);
void MA_free(void *ptr);

#ifdef MA_OVERRIDE_STDLIB
#define malloc(n)	MA_malloc(n)
#define calloc(n, e)	MA_calloc(n, e)
#define realloc(p, n)	MA_realloc(p, n)
#define free(p)		MA_free(p)
#endif

#endif
