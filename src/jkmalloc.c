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

#include "jkmalloc.h"

#ifndef PAGESIZE
#define PAGESIZE jk_pagesize()
static size_t jk_pagesize(void)
{
	static size_t pagesize = 0;
	if (pagesize == 0) {
		pagesize = (size_t)sysconf(_SC_PAGESIZE);
	}
	return pagesize;
}
#endif

#define JKMALLOC_EXIT_VALUE (127 + SIGSEGV)
#define PAGES_PER_TRIE	(1024)
#define TRIE_SIZE	(PAGESIZE * PAGES_PER_TRIE)

struct jk_bucket {
	size_t used;
	size_t allocated;
	void *under;
	void *over;
};

static void *jk_page_alloc(size_t npages)
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

static void jk_sigaction(int sig, siginfo_t *si, void *addr)
{
	(void)sig; (void)addr;

	if (addr == NULL) {
		fprintf(stderr, "NULL pointer dereference\n");
	} else {
		fprintf(stderr, "error accessing %p\n", si->si_addr);
	}
	_exit(JKMALLOC_EXIT_VALUE);
}

static void jk_abort(const char *func, void *ptr)
{
	fprintf(stderr, "%s(): invalid pointer %p\n", func, ptr);
	_exit(JKMALLOC_EXIT_VALUE);
}

static void jk_set_sigaction(void)
{
	struct sigaction sa = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = jk_sigaction,
	};
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
}

static struct jk_bucket *jk_bucket(void *ptr, int allocate)
{
	/* FIXME: check return values of page_alloc() */

	/* FIXME: assumption that one page can hold UCHAR_MAX uintptr_t */
	/*
	static size_t per_trie = 0;
	if (per_trie == 0) {
		per_trie = TRIE_SIZE / sizeof(uintptr_t);
	}
	*/

	static uintptr_t *trie_top = NULL;
	if (trie_top == NULL) {
		trie_top = jk_page_alloc(1);
		memset(trie_top, 0, PAGESIZE);
	}

	jk_set_sigaction();

	uintptr_t *trie = trie_top;
	uintptr_t addr = (uintptr_t)ptr;
	for (size_t i = 0; i < sizeof(addr); i++) {
		uintptr_t next = (addr >> ((sizeof(addr) - i) * CHAR_BIT))
			& UCHAR_MAX;

		if (trie[next] == 0) {
			if (allocate) {
				uintptr_t *newtrie = jk_page_alloc(1);
				memset(newtrie, 0, PAGESIZE);
				trie[next] = (uintptr_t) newtrie;
			} else {
				return NULL;
			}
		}
		trie = (uintptr_t*)trie[next];
	}
	return trie ? (struct jk_bucket *)trie : NULL;
}

void *jk_calloc(size_t nelem, size_t elsize)
{
	size_t n = nelem * elsize;
	if (n < nelem || n < elsize) {
		/* overflow */
		return NULL;
	}
	void *ptr = jk_malloc(n);
	memset(ptr, 0, n);
	return ptr;
}

void *jk_malloc(size_t nbytes)
{
	size_t pages = 2 + (nbytes / PAGESIZE);
	if (nbytes % PAGESIZE != 0) {
		pages++;
	}

	char *ptr = jk_page_alloc(pages);
	if (ptr == MAP_FAILED) {
		return NULL;
	}

	struct jk_bucket *b = jk_bucket(ptr + PAGESIZE, 1);
	b->used = nbytes;
	b->allocated = pages * PAGESIZE;
	b->under = ptr;
	b->over = ptr + ((pages - 1) * PAGESIZE);

	mprotect(b->under, PAGESIZE, PROT_NONE);
	mprotect(b->over, PAGESIZE, PROT_NONE);

	return ptr + PAGESIZE;
}

void *jk_realloc(void *ptr, size_t n)
{
	if (ptr == NULL) {
		return jk_malloc(n);
	}

	struct jk_bucket *b = jk_bucket(ptr, 0);
	if (b == NULL) {
		jk_abort(__func__, ptr);
	}

	if (n < (b->allocated - (PAGESIZE * 2))) {
		b->used = n;
		char *over = (char*)ptr + b->used + (PAGESIZE - (PAGESIZE % b->used));
		if (over != b->over) {
			mprotect(over, PAGESIZE, PROT_NONE);
			munmap(over + PAGESIZE, (char*)b->over - over - PAGESIZE);
			b->over = over;
		}
		return ptr;
	}

	void *newptr = jk_malloc(n);
	if (newptr != NULL) {
		memcpy(newptr, ptr, b->used);
		jk_free(ptr);
	}
	return newptr;
}

void jk_free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	struct jk_bucket *b = jk_bucket(ptr, 0);
	if (b == NULL) {
		jk_abort(__func__, ptr);
	}

	char *base = ptr;
	base -= PAGESIZE;
	munmap(base, b->allocated);

	/* TODO: clear bucket */
}
