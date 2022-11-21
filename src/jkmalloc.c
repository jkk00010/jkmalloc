#define _DEFAULT_SOURCE "give me anonymous, gnu"
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "jkmalloc.h"

#ifndef PAGESIZE
#define PAGESIZE	(4096)
#endif

#define PTR_BITS	(CHAR_BIT * sizeof(uintptr_t))

#define JKMALLOC_EXIT_VALUE	(127 + SIGSEGV)
#define JK_FREE_LIST_SIZE	(8)

/* magic numbers derived from CRC-32 of jk_foo_block */
#define JK_FREE_MAGIC		(0x551a51dc)
#define JK_UNDER_MAGIC		(0xcb2873ac)
#define JK_OVER_MAGIC		(0x18a12c17)

#define jk_pages(bytes)		(((bytes + PAGESIZE - 1) / PAGESIZE) + 2)
#define jk_pageof(addr)		((void*)((uintptr_t)addr - ((uintptr_t)addr % PAGESIZE)))
#define jk_bucketof(addr)	((void*)((uintptr_t)jk_pageof(addr) - PAGESIZE))

struct jk_bucket {
	uint32_t magic;
	uintptr_t start;
	size_t size;
	size_t align;
	size_t pages;
};

static struct jk_bucket *jk_free_list[JK_FREE_LIST_SIZE];
static size_t jk_free_buckets = 0;

static void jk_error(const char *s, void *addr)
{
	if (s) {
		write(STDERR_FILENO, s, strlen(s));
		if (addr != NULL) {
			char hex[] = "01234567890abcdef";
			char ha[sizeof(uintptr_t) * 2 + 5] = ": 0x";
			uintptr_t a = (uintptr_t)addr;
			/* FIXME */
			for (size_t i = 0; i < sizeof(uintptr_t); i++) {
				ha[4 + 2 * i] = hex[(a >> (PTR_BITS - i)) & 0xf];
				ha[4 + 2 * i + 1] = hex[(a >> (PTR_BITS - i + 1)) & 0xf];
			}
			write(STDERR_FILENO, ha, sizeof(ha));
		}
		write(STDERR_FILENO, "\n", 1);
	}
	_exit(JKMALLOC_EXIT_VALUE);
}

static void *jk_page_alloc(size_t npages)
{
	int fd = -1;
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_PRIVATE;

	#ifdef MAP_ANONYMOUS
	flags |= MAP_ANONYMOUS;
	#else
	fd = open("/dev/zero", O_RDONLY);
	#endif

	void *pages = mmap(NULL, npages * PAGESIZE, prot, flags, fd, 0);

	#ifndef MAP_ANONYMOUS
	if (fd != -1) {
		close(fd);
	}
	#endif

	return pages;
}

static void jk_sigaction(int sig, siginfo_t *si, void *addr)
{
	(void)sig; (void)addr;
	if (si->si_addr == NULL) {
		psiginfo(si, "NULL pointer dereference");
		jk_error(NULL, NULL);
	}

	struct jk_bucket *bucket = jk_pageof(si->si_addr);
	if (mprotect(bucket, PAGESIZE, PROT_READ) != 0) {
		psiginfo(si, NULL);
		jk_error(NULL, NULL);
	}

	switch (bucket->magic) {
	case JK_UNDER_MAGIC:
		if (bucket->size == 0) {
			psiginfo(si, "Attempt to use 0-byte allocation");
		} else {
			psiginfo(si, "Heap underflow detected");
		}
		break;

	case JK_OVER_MAGIC:
		if (bucket->size == 0) {
			psiginfo(si, "Attempt to use 0-byte allocation");
		} else {
			psiginfo(si, "Heap overflow detected");
		}
		break;

	case JK_FREE_MAGIC:
		psiginfo(si, "Use after free() detected");
		break;

	default:
		psiginfo(si, NULL);
	}

	jk_error(NULL, NULL);
}

void *jk_calloc(size_t nelem, size_t elsize)
{
	size_t n = nelem * elsize;
	if (n < nelem || n < elsize) {
		/* overflow */
		errno = ENOMEM;
		return NULL;
	}
	void *ptr = jk_malloc(n);
	memset(ptr, 0, n);
	return ptr;
}

int jk_memalign(void **memptr, size_t alignment, size_t size)
{
	if (memptr == NULL) {
		return EINVAL;
	}

	static int sa_set = 0;
	if (!sa_set) {
		struct sigaction sa = {
			.sa_flags = SA_SIGINFO,
			.sa_sigaction = jk_sigaction,
		};
		sigemptyset(&sa.sa_mask);
		sigaction(SIGSEGV, &sa, NULL);
		sa_set = 1;
	}

	size_t pages = jk_pages(size);

	struct jk_bucket *under = jk_page_alloc(pages);
	if (under == MAP_FAILED) {
		errno = ENOMEM;
		return ENOMEM;
	}

	under->magic = JK_UNDER_MAGIC;
	under->size = size;
	under->align = alignment;
	under->pages = pages;
	under->start = (uintptr_t)under + PAGESIZE;
	if (size % PAGESIZE != 0) {
		under->start += PAGESIZE - size % PAGESIZE;
		if (under->start % under->align != 0) {
			under->start -= under->start % under->align;
		}
	}

	struct jk_bucket *over = (void*)((char*)under + PAGESIZE * (pages - 1));
	over->magic = JK_OVER_MAGIC;
	over->start = under->start;
	over->size = under->size;

	*memptr = (void*)under->start;

	mprotect(under, PAGESIZE, PROT_NONE);
	mprotect(over, PAGESIZE, PROT_NONE);

	return 0;
}

void *jk_aligned_alloc(size_t alignment, size_t size)
{
	void *ptr = NULL;
	if (jk_memalign(&ptr, alignment, size) == 0) {
		return ptr;
	}
	return NULL;
}

void *jk_malloc(size_t nbytes)
{
	return jk_aligned_alloc(1, nbytes);
}

void *jk_realloc(void *ptr, size_t n)
{
	if (ptr == NULL) {
		return jk_malloc(n);
	}

	struct jk_bucket *b = jk_bucketof(ptr);
	if (mprotect(b, PAGESIZE, PROT_READ | PROT_WRITE) != 0) {
		jk_error("Attempt to realloc() non-dynamic address", ptr);
	}

	if (b->magic == JK_FREE_MAGIC) {
		jk_error("Attempt to realloc() after free()", ptr);
	}

	if (b->magic != JK_UNDER_MAGIC) {
		jk_error("Attempt to realloc() non-dynamic address", ptr);
	}

	if (b->start != (uintptr_t)ptr) {
		jk_error("Attempt to reallocate() incorrect address", ptr);
	}

	size_t newpages = jk_pages(n);
	if (newpages == b->pages) {
		/* TODO: this is more complex with right-alignment */
		/*
		b->size = n;
		mprotect(b, PAGESIZE, PROT_NONE);
		return ptr;
		*/
	}

	if (newpages < b->pages) {
		/* TODO: mprotect() the newly inaccessible pages */
		/* this is an optimization only */
		/* falling through to malloc(), memcpy(), free() is still correct */
	}

	void *newptr = jk_aligned_alloc(b->align, n);
	if (newptr != NULL) {
		memcpy(newptr, ptr, b->size);
		jk_free(ptr);
	}
	return newptr;
}

void jk_free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	struct jk_bucket *b = jk_bucketof(ptr);
	if (mprotect(b, PAGESIZE, PROT_READ | PROT_WRITE) != 0) {
		jk_error("Attempt to free() non-dynamic address", ptr);
	}

	if (b->magic == JK_FREE_MAGIC) {
		jk_error("Double free() detected", ptr);
	}

	if (b->magic != JK_UNDER_MAGIC) {
		jk_error("Attempt to free() non-dynamic address", ptr);
	}

	if (b->start != (uintptr_t)ptr) {
		jk_error("Attempt to free() incorrect address", ptr);
	}

	char *base = (char*)b;
	mprotect(base, PAGESIZE * b->pages, PROT_READ | PROT_WRITE);

	for (size_t i = 0; i < b->pages; i++) {
		struct jk_bucket *p = (void*)(base + i * PAGESIZE);
		p->magic = JK_FREE_MAGIC;
		p->start = b->start;
		p->size = b->size;
	}

	size_t fb = jk_free_buckets % JK_FREE_LIST_SIZE;
	if (jk_free_buckets > JK_FREE_LIST_SIZE) {
		mprotect(jk_free_list[fb], PAGESIZE, PROT_READ);
		munmap(jk_free_list[fb], PAGESIZE * jk_free_list[fb]->pages);
	}
	jk_free_list[fb] = b;
	jk_free_buckets++;
	mprotect(b, PAGESIZE * b->pages, PROT_NONE);
}
