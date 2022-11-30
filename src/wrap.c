#include "jkmalloc.h"

void *malloc(size_t n)
{
	return jkmalloc(NULL, NULL, 0, NULL, 1, n, 0);
}

void *calloc(size_t n, size_t e)
{
	return jkmalloc(NULL, NULL, 0, NULL, 1, n, e);
}

void *realloc(void *p, size_t n)
{
	return jkmalloc(NULL, NULL, 0, p, 1, n, 0);
}

void free(void *p)
{
	jkmalloc(NULL, NULL, 0, p, 0, 0, 0);
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	return jk_memalign(memptr, alignment, size);
}

void *aligned_alloc(size_t alignment, size_t size)
{
	return jkmalloc(NULL, NULL, 0, NULL, alignment, size, 0);
}
