jkmalloc
--------
jkmalloc is a memory mapping based allocator with an API compatible with
the C `<stdlib.h>` dynamic memory functions. By using memory mappings for each
allocation, jkmalloc is able to provide guard pages to detect heap overflow
and underflow, as well as potentially identifying use-after-free or double-free
errors (in all cases, your program will crash, which is preferable to being
silently exploited).

These guard pages do double duty managing heap metadata. The metadata is
only accessible by jkmalloc functions, so attacks that attempt to overwrite
heap metadata will fail.

Building
--------
Just run `make`. This will give you `libjkmalloc.a`, which you can statically
link into your own programs, as well as `libjkmalloc.so`, which can be used
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
int jk_memalign(void **ptr, size_t align, size_t size);
void *jk_aligned_alloc(size_t align, size_t size);
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
#define malloc(n)		jk_malloc(n)
#define calloc(n, e)		jk_calloc(n, e)
#define realloc(p, n)		jk_realloc(p, n)
#define free(p)			jk_free(p)
#define posix_memalign(p, a, s)	jk_memalign(p, a, s)
#define aligned_alloc(a, s)	jk_aligned_alloc(a, s)
```

Link your program with `-ljkmalloc` (you may also need to specify
`-L` with the path to where `libjkmalloc.a` is if you don't copy it to part
of your linker's default search path).

Wrapper
-------
The dynamic library `libjkmalloc.so` is also built by default. This can
be used to wrap the standard libc functions `malloc()`, `calloc()`, `realloc()`,
`free()`, `posix_memalign()`, and `aligned_alloc()` to their jkmalloc
equivalents if your dynamic linker supports this. For example, on Linux systems
with the GNU linker:

```bash
LD_PRELOAD=libjkmalloc.so command args...
```
