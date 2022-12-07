#ifndef JKMALLOC_H
#define JKMALLOC_H

#include <errno.h>		/* for the definition of errno */
#include <stddef.h>		/* for the definition of size_t and NULL */
#include <stdint.h>		/* for the definition of uintmax_t */

#if defined jk_size || defined jk_align || defined jk_nelem || defined jk_elsize || defined jk_ptr
# error jkmalloc.h conflicts with macros beginning with jk_
#endif

/* general entry point for jkmalloc functionality */
/* this function should not be called directly */
/* instead, use one of the macros defined below */
void* jkmalloc(const char *file, const char *func, uintmax_t line, void *oldptr, size_t alignment, size_t size1, size_t size2);

void *jk_malloc(size_t);
#define jk_malloc(jk_size)			jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_size), 0)

void *jk_aligned_alloc(size_t, size_t);
#define jk_aligned_alloc(jk_align, jk_size)	jkmalloc(__FILE__, __func__, __LINE__, NULL, (jk_align), (jk_size), 0)

void *jk_calloc(size_t, size_t);
#define jk_calloc(jk_nelem, jk_elsize)		jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_nelem), (jk_elsize))

void *jk_realloc(void *, size_t);
#define jk_realloc(jk_ptr, jk_size)		jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 1, (jk_size), 0)

void jk_free(void*);
#define jk_free(jk_ptr)				(void)jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 0, 0, 0)

int jk_memalign(void **, size_t, size_t);
#define jk_memalign(jk_ptr, jk_align, jk_size)	((jk_ptr) == NULL ? EINVAL : (*(jk_ptr) = jkmalloc(__FILE__, __func__, __LINE__, NULL, (jk_align), (jk_size), 0)) != NULL ? 0 : errno)

#ifdef JK_OVERRIDE_STDLIB
#include <stdlib.h>		/* to ensure it has been included */

# undef malloc
# define malloc(jk_size)		jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_size), 0)

# undef calloc
# define calloc(jk_nelem, jk_elsize)	jkmalloc(__FILE__, __func__, __LINE__, NULL, 1, (jk_nelem), (jk_elsize))

# undef realloc
# define realloc(jk_ptr, jk_size)	jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 1, (jk_size), 0)

# undef free
# define free(jk_ptr)			(void)jkmalloc(__FILE__, __func__, __LINE__, (jk_ptr), 0, 0, 0)

# undef posix_memalign
# define posix_memalign(jk_p, jk_a, jk_s)	((jk_ptr) == NULL ? EINVAL : (*(jk_ptr) = jkmalloc(__FILE__, __func__, __LINE__, NULL, (jk_align), (jk_size), 0)) != NULL ? 0 : errno)

# undef aligned_alloc
# define aligned_alloc(jk_align, jk_size)	jkmalloc(__FILE__, __func__, __LINE__, NULL, (jk_align), (jk_size), 0)
#endif

#endif
