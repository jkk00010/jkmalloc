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
	(void)jkmalloc(NULL, NULL, 0, p, 0, 0, 0);
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	if (memptr == NULL) {
		return EINVAL;
	}
	if ((*memptr = jkmalloc(NULL, NULL, 0, NULL, alignment, size, 0)) == NULL) {
		return errno;
	}
	return 0;
}

void *aligned_alloc(size_t alignment, size_t size)
{
	return jkmalloc(NULL, NULL, 0, NULL, alignment, size, 0);
}
