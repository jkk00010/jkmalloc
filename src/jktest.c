#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jkmalloc.h"

#if defined __GNUC__ && !defined __clang__
#pragma GCC diagnostic ignored "-Wuse-after-free"
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

int main(int argc, char *argv[])
{
	char string[] = "test string";

	uintmax_t over = 0;
	uintmax_t under = 0;
	int doublefree = 0;
	int invalidfree = 0;
	int invalidrealloc = 0;
	int null = 0;
	int useafterfree = 0;
	size_t size = sizeof(string);
	uintmax_t randomalloc = 0;
	uintmax_t sequential = 0;

	int c;
	while ((c = getopt(argc, argv, "o:u:dfrnazR:S:")) != -1) {
		switch (c) {
		case 'o':
			over = strtoumax(optarg, NULL, 0);
			break;

		case 'u':
			under = strtoumax(optarg, NULL, 0);
			break;

		case 'd':
			doublefree = 1;
			break;

		case 'f':
			invalidfree = 1;
			break;

		case 'r':
			invalidrealloc = 1;
			break;

		case 'n':
			null = 1;
			break;

		case 'a':
			useafterfree = 1;
			break;

		case 'z':
			size = 0;
			break;

		case 'R':
			randomalloc = strtoumax(optarg, NULL, 0);
			break;

		case 'S':
			sequential = strtoumax(optarg, NULL, 0);
			break;

		default:
			fprintf(stderr, "usage: %s [-o over] [-u under] [-dfrnaz]\n", argv[0]);
			return 1;
		}
	}

	if (randomalloc) {
		for (uintmax_t i = 0; i < randomalloc; i++) {
			void *ptr = malloc(1);
			printf("%p\n", ptr);
			free(ptr);
		}
	}

	if (sequential) {
		void *a[sequential];
		for (uintmax_t i = 0; i < sequential; i++) {
			a[i] = malloc(1);
			printf("%p\n", a[i]);
		}
		for (uintmax_t i = 0; i < sequential; i++) {
			free(a[i]);
		}
	}

	char *ptr = malloc(size);	// allocates 0 if -z specified

	strcpy(ptr, string);		// crash if -z

	if (over) {
		for (uintmax_t i = 0; i < over; i++) {
			ptr[i + size] = string[i % sizeof(string)];
		}
	}

	if (under) {
		for (uintmax_t i = 0; i < under; i++) {
			ptr[-i] = string[i % sizeof(string)];
		}
	}

	free(ptr);

	if (useafterfree) {
		puts(ptr);
	}

	if (doublefree) {
		free(ptr);
	}

	if (invalidfree) {
		ptr = string;
		free(ptr);
	}

	if (invalidrealloc) {
		ptr = realloc(string, size);
	}

	if (null) {
		ptr = NULL;
		strcpy(ptr, string);
		puts(ptr);
	}
}
