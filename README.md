MapAlloc
--------
MapAlloc is a memory mapping based allocator with an API compatible with
the C `<stdlib.h>` dynamic memory functions. By using memory mappings for each
allocation, MapAlloc is able to provide guard pages to detect heap overflow
and underflow, as well as potentially identifying use-after-free or double-free
errors (in all cases, your program will crash, which is preferable to being
silently exploited).

MapAlloc also maintains metadata independently from the heap, so it eliminates
an entire class of vulnerabilities which rely on easy access to this metadata.

Building
--------
Just run `make`. This will give you `libmapalloc.a`, which you can statically
link into your own programs, as well as `libwrapalloc.so`, which can be used
with `LD_PRELOAD` to override your standard C library's dynamic memory
functions.

Including in your program
-------------------------
In your source file:

```c
#include "mapalloc.h"
```

This will give you:

```c
void *MA_malloc(size_t n);
void *MA_calloc(size_t nelem, size_t elsize);
void *MA_realloc(void *ptr, size_t n);
void MA_free(void *ptr);
```

Or, you can ask for macros to provide the same interfaces as `<stdlib.h>`
(note that if you need to also include `<stdlib.h>`, you should include it
before `"mapalloc.h"`):

```c
#define MA_OVERRIDE_STDLIB
#include "mapalloc.h"
```

This will give you access to the same set of functions as above, but also
provide macros:

```c
#define malloc(n)	MA_malloc(n)
#define calloc(n, e)	MA_calloc(n, e)
#define realloc(p, n)	MA_realloc(p, n)
#define free(p)		MA_free(p)
```

Link your program with `-lmapalloc` (you may also need to specify
`-L` with the path to where `libmapalloc.a` is if you don't copy it to part
of your linker's default search path).
