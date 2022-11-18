#ifndef JKMALLOC_H
#define JKMALLOC_H

#include <stddef.h>		/* for the definition of size_t */

void *jk_malloc(size_t n);
void *jk_calloc(size_t nelem, size_t elsize);
void *jk_realloc(void *ptr, size_t n);
void jk_free(void *ptr);

#ifdef JK_OVERRIDE_STDLIB
#undef malloc
#define malloc(n)	jk_malloc(n)

#undef calloc
#define calloc(n, e)	jk_calloc(n, e)

#undef realloc
#define realloc(p, n)	jk_realloc(p, n)

#undef free
#define free(p)		jk_free(p)
#endif

#endif
