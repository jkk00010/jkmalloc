#define _DEFAULT_SOURCE "give me anonymous, gnu"
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "jkmalloc.h"

#if defined __OpenBSD__ || defined __FreeBSD__ || defined __APPLE__
#define psiginfo(x, y)	((y) ? fprintf(stderr, "%s\n", (char*)(y)) : 0)
#ifndef SA_SIGINFO
#define SA_SIGINFO	(0)
#endif
#endif

#define PTR_BITS	(CHAR_BIT * sizeof(uintptr_t))

#define JKMALLOC_EXIT_VALUE	(127 + SIGSEGV)
#define JK_FREE_LIST_SIZE	(8)

/* magic numbers derived from CRC-32 of jk_foo_block */
#define JK_FREE_MAGIC		(0x551a51dc)
#define JK_UNDER_MAGIC		(0xcb2873ac)
#define JK_OVER_MAGIC		(0x18a12c17)

#define jk_pages(bytes)		(((bytes + jk_pagesize - 1) / jk_pagesize) + 2)
#define jk_pageof(addr)		((void*)((uintptr_t)addr - ((uintptr_t)addr % jk_pagesize)))
#define jk_bucketof(addr)	((void*)((uintptr_t)jk_pageof(addr) - jk_pagesize))

struct jk_bucket {
	uint32_t magic;
	uintptr_t start;
	size_t size;
	size_t align;
	size_t pages;
	size_t tlen;
	char trace[];
};

struct jk_source {
	const char *file;
	const char *func;
	uintmax_t line;
	struct jk_bucket *bucket;
};

static struct jk_bucket *jk_free_list[JK_FREE_LIST_SIZE];
static size_t jk_free_buckets = 0;
static size_t jk_pagesize = 0;

static void jk_copy(void *dst, void *src, size_t len, int fromtop)
{
	char *d = dst;
	char *s = src;
	if (fromtop) {
		for (size_t i = 1; i <= len; i++) {
			d[len - i] = s[len - i];
		}
	} else {
		for (size_t i = 0; i < len; i++) {
			d[i] = s[i];
		}
	}
}

static void jk_error(const char *s, void *addr, struct jk_source *src)
{
	if (s && *s) {
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

	if (src && src->bucket && src->bucket->trace[0] != '\0') {
		write(STDERR_FILENO, src->bucket->trace, src->bucket->tlen);
		write(STDERR_FILENO, "\n", 1);
	}

	if (src && src->file) {
		fprintf(stderr, "!!! %s() (%s:%ju)\n", src->func, src->file, src->line);
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

	void *pages = mmap(NULL, npages * jk_pagesize, prot, flags, fd, 0);

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
		jk_error(NULL, NULL, NULL);
	}

	struct jk_bucket *bucket = jk_pageof(si->si_addr);
	if (mprotect(bucket, jk_pagesize, PROT_READ) != 0) {
		psiginfo(si, NULL);
		jk_error(NULL, NULL, NULL);
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
			fprintf(stderr, "Allocation of size %zu at %p, overflow at %p (offset %zu)\n", bucket->size, (void*)bucket->start, si->si_addr, (size_t)((char*)si->si_addr - (char*)bucket->start));
			fprintf(stderr, "Buffer begins with %4s\n", (char*)bucket->start);
		}
		break;

	case JK_FREE_MAGIC:
		psiginfo(si, "Use after free() detected");
		break;

	default:
		psiginfo(si, NULL);
	}

	struct jk_source src = { .bucket = bucket };
	jk_error(NULL, NULL, &src);
}

void* jkmalloc(const char *file, const char *func, uintmax_t line, void *ptr, size_t alignment, size_t size1, size_t size2)
{
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

	if (jk_pagesize == 0) {
		jk_pagesize = sysconf(_SC_PAGESIZE);
	}

	struct jk_source src = {
		.file = file,
		.func = func,
		.line = line,
	};

	/* free() */
	if (alignment == 0) {
		if (ptr == NULL) {
			return NULL;
		}

		/* TODO: Add source line information to the following errors */

		struct jk_bucket *b = jk_bucketof(ptr);
		if (mprotect(b, jk_pagesize, PROT_READ | PROT_WRITE) != 0) {
			jk_error("Attempt to free() non-dynamic address", ptr, &src);
		}

		src.bucket = b;

		if (b->magic == JK_FREE_MAGIC) {
			jk_error("Double free() detected", ptr, &src);
		}

		if (b->magic != JK_UNDER_MAGIC) {
			jk_error("Attempt to free() non-dynamic address", ptr, &src);
		}

		if (b->start != (uintptr_t)ptr) {
			jk_error("Attempt to free() incorrect address", ptr, &src);
		}

		char *base = (char*)b;
		mprotect(base, jk_pagesize * b->pages, PROT_READ | PROT_WRITE);

		if (file) {
			size_t len = b->tlen;
			b->tlen += snprintf(b->trace + len, jk_pagesize - sizeof(*b) - len,
				"%s--- %s() (%s:%ju)", len ? "\n" : "", func, file, line);
		}

		b->magic = JK_FREE_MAGIC;

		for (size_t i = 1; i < b->pages; i++) {
			jk_copy(base + i * jk_pagesize, b, jk_pagesize, 0);
		}

		size_t fb = jk_free_buckets % JK_FREE_LIST_SIZE;
		if (jk_free_buckets > JK_FREE_LIST_SIZE) {
			mprotect(jk_free_list[fb], jk_pagesize, PROT_READ);
			munmap(jk_free_list[fb], jk_pagesize * jk_free_list[fb]->pages);
		}
		jk_free_list[fb] = b;
		jk_free_buckets++;
		mprotect(b, jk_pagesize * b->pages, PROT_NONE);
		return NULL;
	}

	/* realloc() */
	if (ptr) {

		/* TODO: Add source line information to the following errors */

		struct jk_bucket *b = jk_bucketof(ptr);
		if (mprotect(b, jk_pagesize, PROT_READ | PROT_WRITE) != 0) {
			jk_error("Attempt to realloc() non-dynamic address", ptr, &src);
		}

		src.bucket = b;

		if (b->magic == JK_FREE_MAGIC) {
			jk_error("Attempt to realloc() after free()", ptr, &src);
		}

		if (b->magic != JK_UNDER_MAGIC) {
			jk_error("Attempt to realloc() non-dynamic address", ptr, &src);
		}

		if (b->start != (uintptr_t)ptr) {
			jk_error("Attempt to reallocate() incorrect address", ptr, &src);
		}
	
		void *newptr = jkmalloc(NULL, NULL, 0, NULL, alignment, size1, size2);
		if (newptr != NULL) {
			jk_copy(newptr, ptr, b->size, 0);
			jk_free(ptr);
		}
		return newptr;
	}

	size_t size = size1;

	/* calloc() */
	if (size2) {
		size = size1 * size2;
		if (size < size1 || size < size2) {
			/* overflow */
			errno = ENOMEM;
			return NULL;
		}
	}

	size_t pages = jk_pages(size);

	struct jk_bucket *under = jk_page_alloc(pages);
	if (under == MAP_FAILED) {
		errno = ENOMEM;
		return NULL;
	}

	under->magic = JK_UNDER_MAGIC;
	under->size = size;
	under->align = alignment;
	under->pages = pages;
	under->start = (uintptr_t)under + jk_pagesize;
	if (size % jk_pagesize != 0) {
		under->start += jk_pagesize - size % jk_pagesize;
		if (under->start % under->align != 0) {
			under->start -= under->start % under->align;
		}
	}

	struct jk_bucket *over = (void*)((char*)under + jk_pagesize * (pages - 1));
	over->magic = JK_OVER_MAGIC;
	over->start = under->start;
	over->size = under->size;

	ptr = (void*)under->start;

	if (file) {
		under->tlen = snprintf(under->trace, jk_pagesize - sizeof(*under), "+++ %s() (%s:%ju)", func, file, line);
		jk_copy(over->trace, under->trace, under->tlen + 1, 0);
		over->tlen = under->tlen;
	} else {
		under->trace[0] = '\0';
		over->trace[0] = '\0';
	}

	/* calloc() */
	if (size2) {
		char *p = ptr;
		for (size_t i = 0; i < size; i++) {
			p[i] = '\0';
		}
	}

	mprotect(under, jk_pagesize, PROT_NONE);
	mprotect(over, jk_pagesize, PROT_NONE);
	return ptr;
}

void *(jk_malloc)(size_t n)
{
	return jkmalloc(NULL, NULL, 0, NULL, 1, n, 0);
}

void *(jk_aligned_alloc)(size_t a, size_t n)
{
	return jkmalloc(NULL, NULL, 0, NULL, a, n, 0);
}

void *(jk_calloc)(size_t n, size_t s)
{
	return jkmalloc(NULL, NULL, 0, NULL, 1, n, s);
}

void *(jk_realloc)(void *ptr, size_t n)
{
	return jkmalloc(NULL, NULL, 0, ptr, 1, n, 0);
}

void (jk_free)(void *ptr)
{
	(void)jkmalloc(NULL, NULL, 0, ptr, 0, 0, 0);
}

int (jk_memalign)(void **ptr, size_t a, size_t n)
{
	if (ptr == NULL) {
		return EINVAL;
	}

	if (((*ptr) = jkmalloc(NULL, NULL, 0, NULL, a, n, 0)) == NULL) {
		return errno;
	}

	return 0;
}
