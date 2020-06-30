#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "mapalloc.h"

#ifndef PAGESIZE
#define PAGESIZE get_pagesize()
static size_t get_pagesize(void)
{
	static size_t pagesize = 0;
	if (pagesize == 0) {
		pagesize = (size_t)sysconf(_SC_PAGESIZE);
	}
	return pagesize;
}
#endif

#define MAPALLOC_EXIT_VALUE (127 + SIGSEGV)

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

static void map_signal_action(int sig, siginfo_t *si, void *addr)
{
	(void)sig; (void)addr;
	fprintf(stderr, "error accessing %p\n", si->si_addr);
	_exit(MAPALLOC_EXIT_VALUE);
}

static void set_signal_handler(void)
{
	struct sigaction sa = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = map_signal_action,
	};
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
}

static struct bucket *get_bucket(void *ptr, int allocate)
{
	/* FIXME: assumption that one page can hold UCHAR_MAX uintptr_t */
	/* FIXME: check return values of page_alloc() */

	static uintptr_t *trie_top = NULL;
	if (trie_top == NULL) {
		trie_top = page_alloc(1);
		memset(trie_top, 0, PAGESIZE);
	}

	set_signal_handler();

	uintptr_t *trie = trie_top;
	uintptr_t addr = (uintptr_t)ptr;
	for (size_t i = 0; i < sizeof(addr); i++) {
		uintptr_t next = (addr >> ((sizeof(addr) - i) * CHAR_BIT))
			& UCHAR_MAX;

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
	if (ptr == MAP_FAILED) {
		return NULL;
	}

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
		fprintf(stderr, "%s(%p, %zu): invalid pointer\n", __func__, ptr, n);
		_exit(MAPALLOC_EXIT_VALUE);
	}

	if (n < (b->allocated - (PAGESIZE * 2))) {
		b->used = n;
		/* TODO: munmap() and mprotect() as necessary */
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
		fprintf(stderr, "%s(%p): invalid pointer\n", __func__, ptr);
		_exit(MAPALLOC_EXIT_VALUE);
	}

	char *base = ptr;
	base -= PAGESIZE;
	munmap(base, b->allocated);

	/* TODO: clear bucket */
}
