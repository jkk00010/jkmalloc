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
