#ifndef JKMALLOC_H
#define JKMALLOC_H

#include <stddef.h>		/* for the definition of size_t */

void *jk_malloc(size_t n);
void *jk_calloc(size_t nelem, size_t elsize);
void *jk_realloc(void *ptr, size_t n);
void jk_free(void *ptr);

#ifdef JK_OVERRIDE_STDLIB
#undef malloc
#define malloc(jk_n)		jk_malloc(jk_n)

#undef calloc
#define calloc(jk_n, jk_e)	jk_calloc(jk_n, jk_e)

#undef realloc
#define realloc(jk_p, jk_n)	jk_realloc(jk_p, jk_n)

#undef free
#define free(jk_p)		jk_free(jk_p)
#endif

#endif
