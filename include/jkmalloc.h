#ifndef JKMALLOC_H
#define JKMALLOC_H

#include <stddef.h>		/* for the definition of size_t */

void *jk_malloc(size_t);
void *jk_calloc(size_t, size_t);
void *jk_realloc(void *, size_t);
void jk_free(void *);
int jk_memalign(void **, size_t, size_t);
void *jk_aligned_alloc(size_t, size_t);

#ifdef JK_OVERRIDE_STDLIB
# undef malloc
# define malloc(jk_n)		jk_malloc(jk_n)

# undef calloc
# define calloc(jk_n, jk_e)	jk_calloc(jk_n, jk_e)

# undef realloc
# define realloc(jk_p, jk_n)	jk_realloc(jk_p, jk_n)

# undef free
# define free(jk_p)		jk_free(jk_p)

# undef posix_memalign
# define posix_memalign(jk_p, jk_a, jk_s)	jk_memalign(jk_p, jk_a, jk_s)

# undef aligned_alloc
# define aligned_alloc(jk_a, jk_s)		jk_aligned_alloc(jk_a, jk_s)
#endif

#endif
