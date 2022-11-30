#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define JK_OVERRIDE_STDLIB
#include "jkmalloc.h"

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

	int c;
	while ((c = getopt(argc, argv, "o:u:dfrnaz")) != -1) {
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

		default:
			fprintf(stderr, "usage: %s [-o over] [-u under] [-dfrnaz]\n", argv[0]);
			return 1;
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
		free(string);
	}

	if (invalidrealloc) {
		ptr = realloc(string, size);
	}

	if (null) {
		*ptr = *((char*)NULL);
	}
}
