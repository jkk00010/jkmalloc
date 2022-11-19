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
#define PAGESIZE 4096
#endif

#define JKMALLOC_EXIT_VALUE (127 + SIGSEGV)
#define BUCKETS_PER_NODE	((PAGESIZE - sizeof(struct jk_list)) / sizeof(struct jk_bucket))

struct jk_bucket {
	enum { UNUSED, FREE, ALLOCATED } state;
	uintptr_t start;
	size_t size;
};

struct jk_list {
	struct jk_list *next;
	struct jk_bucket b[];
};

static struct jk_list *jk_head = NULL;

static void jk_error(const char *s, void *addr)
{
	write(STDOUT_FILENO, s, strlen(s));
	if (addr != NULL) {
		char hex[] = "01234567890abcdef";
		char ha[sizeof(uintptr_t) * 2 + 5] = ": 0x";
		uintptr_t a = (uintptr_t)addr;
		for (size_t i = 0; i < sizeof(uintptr_t); i++) {
			ha[4 + 2 * i] = hex[a & 0xf];
			ha[4 + 2 * i + 1] = hex[a & 0xf];
		}
		write(STDOUT_FILENO, ha, sizeof(ha));
	}
	write(STDOUT_FILENO, "\n", 1);
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

	if (fd != -1) {
		close(fd);
	}

	return pages;
}

#define jk_pages(bytes) (((bytes + PAGESIZE - 1) / PAGESIZE) + 2)

static struct jk_bucket *jk_bucket(void *addr)
{
	uintptr_t a = (uintptr_t)addr;
	for (struct jk_list *l = jk_head; l != NULL; l = l->next) {
		for (size_t i = 0; i < BUCKETS_PER_NODE; i++) {
			uintptr_t bot = l->b[i].start - PAGESIZE;
			uintptr_t top = l->b[i].start + (jk_pages(l->b[i].size) + 1) * PAGESIZE;
			if (bot <= a && a <= top) {
				return l->b + i;
			}
		}
	}

	return NULL;
}

static void jk_sigaction(int sig, siginfo_t *si, void *addr)
{
	(void)sig; (void)addr;
	struct jk_bucket *bucket = jk_bucket(si->si_addr);

	if (!bucket || bucket->start == 0) {
		psiginfo(si, NULL);
		if (si->si_addr == NULL) {
			jk_error("NULL pointer dereference", NULL);
		}
	} else {
		psiginfo(si, "Heap Error");
		if (bucket->state == FREE) {
			jk_error("Use after free() detected", si->si_addr);
		} else if (bucket->size == 0) {
			jk_error("Attempt to use 0-byte allocation", si->si_addr);
		} else if ((uintptr_t)si->si_addr < bucket->start) {
			jk_error("Heap underflow detected", si->si_addr);
		} else if ((uintptr_t)si->si_addr > bucket->start + bucket->size) {
			jk_error("Heap overflow detected", si->si_addr);
		}
	}
	_exit(JKMALLOC_EXIT_VALUE);
}

static void jk_set_sigaction(void)
{
	static int set = 0;
	if (set) {
		return;
	}

	struct sigaction sa = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = jk_sigaction,
	};
	sigemptyset(&sa.sa_mask);
	sigaction(SIGSEGV, &sa, NULL);
	set = 1;
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

void *jk_malloc(size_t nbytes)
{
	jk_set_sigaction();

	size_t pages = jk_pages(nbytes);

	char *ptr = jk_page_alloc(pages);
	if (ptr == MAP_FAILED) {
		errno = ENOMEM;
		return NULL;
	}

	char *start = ptr + PAGESIZE;

	struct jk_bucket *b = jk_bucket(ptr);
	if (b == NULL) {
		//b = jk_add_bucket(ptr);
		if (jk_head == NULL) {
			jk_head = jk_page_alloc(1);
		}

		for (struct jk_list *l = jk_head; l != NULL; l = l->next) {
			for (size_t i = 0; i < BUCKETS_PER_NODE; i++) {
				if (l->b[i].state == UNUSED) {
					b = l->b + i;
					break;
				}
			}
			if (b != NULL) {
				break;
			}
			if (l->next == NULL) {
				l->next = jk_page_alloc(1);
			}
		}
	}

	if (b == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	b->state = ALLOCATED;
	b->start = (uintptr_t)start;
	b->size = nbytes;

	void *under = ptr;
	void *over = ptr + ((pages - 1) * PAGESIZE);

	mprotect(under, PAGESIZE, PROT_NONE);
	mprotect(over, PAGESIZE, PROT_NONE);

	return ptr + PAGESIZE;
}

void *jk_realloc(void *ptr, size_t n)
{
	if (ptr == NULL) {
		return jk_malloc(n);
	}

	struct jk_bucket *b = jk_bucket(ptr);
	if (b == NULL || b->start != (uintptr_t)ptr || b->state != ALLOCATED) {
		jk_error("Attempt to realloc() non-dynamic address", ptr);
	}

	size_t newpages = jk_pages(n);
	size_t oldpages = jk_pages(b->size);
	if (newpages == oldpages) {
		b->size = n;
		return ptr;
	}

	if (newpages < oldpages) {
		/* TODO: mprotect() the newly inaccessible pages */
		/* this is an optimazation only */
		/* falling through to malloc(), memcpy(), free() is still correct */
	}

	void *newptr = jk_malloc(n);
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

	struct jk_bucket *b = jk_bucket(ptr);
	if (b == NULL || b->start != (uintptr_t)ptr) {
		jk_error("Attempt to free() non-dynamic address", ptr);
	}

	if (b->state == FREE) {
		jk_error("Attempt to double free()", ptr);
	}

	if (b->state != ALLOCATED) {
		jk_error("Attempt to free() non-allocated address", ptr);
	}

	char *base = (char*)b->start - PAGESIZE;
	munmap(base, PAGESIZE * jk_pages(b->size));
	b->state = FREE;

}
