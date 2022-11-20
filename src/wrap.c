#include "jkmalloc.h"

void *malloc(size_t n)
{
	return jk_malloc(n);
}

void *calloc(size_t n, size_t e)
{
	return jk_calloc(n, e);
}

void *realloc(void *p, size_t n)
{
	return jk_realloc(p, n);
}

void free(void *p)
{
	jk_free(p);
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	return jk_memalign(memptr, alignment, size);
}

void *aligned_alloc(size_t alignment, size_t size)
{
	return jk_aligned_alloc(alignment, size);
}
