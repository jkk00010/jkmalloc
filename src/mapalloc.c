#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "mapalloc.h"

#ifndef PAGESIZE
static size_t get_pagesize(void)
{
	static size_t pagesize = 0;
	if (pagesize == 0) {
		pagesize = (size_t)sysconf(_SC_PAGESIZE);
	}
	return pagesize;
}

#define PAGESIZE get_pagesize()
#endif

struct bucket {
	size_t used;
	size_t allocated;
};

static void *page_alloc(size_t npages)
{
	int fd = -1;
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_PRIVATE;

	#ifdef MAP_ANONYMOUS
	flags = MAP_ANONYMOUS;
	#else
	fd = open("/dev/zero", O_RDONLY);
	#endif

	void *pages = mmap(NULL, npages * PAGESIZE, prot, flags, fd, 0);

	if (fd != -1) {
		close(fd);
	}

	return pages;
}

static struct bucket *get_bucket(void *ptr, int allocate)
{
	static uintptr_t *trie_top = NULL;
	if (trie_top == NULL) {
		trie_top = page_alloc(1);
		memset(trie_top, 0, PAGESIZE);
	}

	printf("- finding bucket %p (%d)\n", ptr, allocate);
	uintptr_t *trie = trie_top;
	uintptr_t addr = (uintptr_t)ptr;
	for (size_t i = 0; i < sizeof(addr); i++) {
		uintptr_t next = (addr >> ((sizeof(addr) - i) * CHAR_BIT))
			& UCHAR_MAX;

		printf("-- %02zx\n", next);
		if (trie[next] == 0) {
			if (allocate) {
				uintptr_t *newtrie = page_alloc(1);
				memset(newtrie, 0, PAGESIZE);
				trie[next] = (uintptr_t) newtrie;
			} else {
				return NULL;
			}
		}
		trie = (uintptr_t*)trie[next];
	}
	return trie ? (struct bucket *)trie : NULL;
}

void *map_calloc(size_t nelem, size_t elsize)
{
	size_t n = nelem * elsize;
	if (n < nelem || n < elsize) {
		/* overflow */
		return NULL;
	}
	void *ptr = map_malloc(n);
	memset(ptr, 0, n);
	return ptr;
}

void *map_malloc(size_t nbytes)
{
	size_t pages = 2 + (nbytes / PAGESIZE);
	if (nbytes % PAGESIZE != 0) {
		pages++;
	}

	char *ptr = page_alloc(pages);

	mprotect(ptr, PAGESIZE, PROT_NONE);
	mprotect(ptr + ((pages - 1) * PAGESIZE), PAGESIZE, PROT_NONE);

	struct bucket *b = get_bucket(ptr + PAGESIZE, 1);
	b->used = nbytes;
	b->allocated = pages * PAGESIZE;

	return ptr + PAGESIZE;
}

void *map_realloc(void *ptr, size_t n)
{
	if (ptr == NULL) {
		return map_malloc(n);
	}

	struct bucket *b = get_bucket(ptr, 0);
	if (b == NULL) {
		fprintf(stderr, "attempt to realloc() invalid pointer %p\n", ptr);
		abort();
	}

	if (n < (b->allocated - (PAGESIZE * 2))) {
		b->used = n;
		/* munmap() and mprotect() as necessary */
		return ptr;
	}

	void *newptr = map_malloc(n);
	if (newptr != NULL) {
		memcpy(newptr, ptr, b->used);
		map_free(ptr);
	}
	return newptr;
}

void map_free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	struct bucket *b = get_bucket(ptr, 0);
	if (b == NULL) {
		fprintf(stderr, "attempt to free() invalid pointer %p\n", ptr);
		abort();
	}

	char *base = ptr;
	base -= PAGESIZE;
	munmap(base, b->allocated);

	/* clear bucket */
}
