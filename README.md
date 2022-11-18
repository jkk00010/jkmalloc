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
Just run `make`. This will give you `libjkmalloc.a`, which you can statically
link into your own programs, as well as `libwrapalloc.so`, which can be used
with `LD_PRELOAD` to override your standard C library's dynamic memory
functions.

Including in your program
-------------------------
In your source file:

```c
#include "jkmalloc.h"
```

This will give you:

```c
void *jk_malloc(size_t n);
void *jk_calloc(size_t nelem, size_t elsize);
void *jk_realloc(void *ptr, size_t n);
void jk_free(void *ptr);
```

Or, you can ask for macros to provide the same interfaces as `<stdlib.h>`
(note that if you need to also include `<stdlib.h>`, you should include it
before `"jkmalloc.h"`):

```c
#define JK_OVERRIDE_STDLIB
#include "jkmalloc.h"
```

This will give you access to the same set of functions as above, but also
provide macros:

```c
#define malloc(n)	jk_malloc(n)
#define calloc(n, e)	jk_calloc(n, e)
#define realloc(p, n)	jk_realloc(p, n)
#define free(p)		jk_free(p)
```

Link your program with `-ljkmalloc` (you may also need to specify
`-L` with the path to where `libjkmalloc.a` is if you don't copy it to part
of your linker's default search path).

Wrapper
-------
The dynamic library `libwrapalloc.so` is also built by default. This can
be used to wrap the standard libc functions `malloc()`, `calloc()`, `realloc()`,
and `free()` to their MapAlloc equivalents if your dynamic linker supports
this. For example, on Linux systems with the GNU linker:

```bash
LD_PRELOAD=libwrapalloc.so command args...
```
