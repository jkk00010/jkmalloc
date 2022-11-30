#ifndef JKMALLOC_H
#define JKMALLOC_H

#include <stddef.h>		/* for the definition of size_t */
#include <stdint.h>		/* for the definition of uintmax_t */

/* general entry point for jkmalloc functionality */
/* this function should not be called directly */
/* instead, use one of the macros defined below */
void* jkmalloc(const char *file, const char *func, uintmax_t line, void *oldptr, size_t alignment, size_t size1, size_t size2);

#define jk_malloc(jk_size)			jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_size), 0)
#define jk_aligned_alloc(jk_align, jk_size)	jkmalloc(__FILE__, __func__, __LINE__, NULL, (jk_align), (jk_size), 0)
#define jk_calloc(jk_nelem, jk_elsize)		jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_nelem), (jk_elsize))
#define jk_realloc(jk_ptr, jk_size)		jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 1, (jk_size), 0)
#define jk_free(jk_ptr)				(void)jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 0, 0, 0)

int jk_memalign(void **, size_t, size_t);

#ifdef JK_OVERRIDE_STDLIB
# undef malloc
# define malloc(jk_size)		jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_size), 0)

# undef calloc
# define calloc(jk_nelem, jk_elsize)	jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_nelem), (jk_elsize))

# undef realloc
# define realloc(jk_ptr, jk_size)	jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 1, (jk_size), 0)

# undef free
# define free(jk_ptr)			(void)jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 0, 0, 0)

# undef posix_memalign
# define posix_memalign(jk_p, jk_a, jk_s)	jk_memalign(jk_p, jk_a, jk_s)

# undef aligned_alloc
# define aligned_alloc(jk_align, jk_size)	jkmalloc(__FILE__, __func__, __LINE__, NULL, (jk_align), (jk_size), 0)
#endif

#endif
