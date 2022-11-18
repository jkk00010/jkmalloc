.POSIX:

BINDIR=bin
LIBDIR=lib
OBJDIR=obj
SRCDIR=src
INCDIR=include
TESTDIR=test
CC=c99
CFLAGS=-I$(INCDIR) -Wall -Wextra -Wpedantic -g -fPIC

OBJECTS=$(OBJDIR)/jkmalloc.o
WRAPOBJECTS=$(OBJECTS) $(OBJDIR)/wrap.o
TESTS=$(BINDIR)/overflow \
	$(BINDIR)/underflow \
	$(BINDIR)/zero-alloc \
	$(BINDIR)/realloc \
	$(BINDIR)/use-after-free \
	$(BINDIR)/double-free \
	$(BINDIR)/invalid-free \
	$(BINDIR)/invalid-realloc \
	$(BINDIR)/wrapper \
	$(BINDIR)/null
JKTESTS=$(BINDIR)/jk-overflow \
	$(BINDIR)/jk-underflow \
	$(BINDIR)/jk-zero-alloc \
	$(BINDIR)/jk-realloc \
	$(BINDIR)/jk-use-after-free \
	$(BINDIR)/jk-double-free \
	$(BINDIR)/jk-macros \
	$(BINDIR)/jk-invalid-free \
	$(BINDIR)/jk-invalid-realloc \
	$(BINDIR)/jk-null

tests: all $(TESTS) $(JKTESTS)

all: $(LIBDIR)/libjkmalloc.a $(LIBDIR)/libjkmalloc.so

$(OBJDIR)/jkmalloc.o: $(SRCDIR)/jkmalloc.c $(INCDIR)/jkmalloc.h
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/$(*F).c

$(OBJDIR)/wrap.o: $(SRCDIR)/wrap.c $(INCDIR)/jkmalloc.h
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/$(*F).c

$(LIBDIR)/libjkmalloc.a: $(OBJECTS)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $<

$(LIBDIR)/libjkmalloc.so: $(WRAPOBJECTS)
	@mkdir -p $(@D)
	$(CC) -o $@ -shared $(WRAPOBJECTS)

$(TESTS): $(LIBDIR)/libjkmalloc.a
$(BINDIR)/overflow: $(TESTDIR)/overflow.c
$(BINDIR)/underflow: $(TESTDIR)/underflow.c
$(BINDIR)/zero: $(TESTDIR)/zero.c
$(BINDIR)/realloc: $(TESTDIR)/realloc.c
$(BINDIR)/use-after-free: $(TESTDIR)/use-after-free.c
$(BINDIR)/double-free: $(TESTDIR)/double-free.c
$(BINDIR)/invalid-free: $(TESTDIR)/invalid-free.c
$(BINDIR)/invalid-realloc: $(TESTDIR)/invalid-realloc.c
$(BINDIR)/wrapper: $(TESTDIR)/wrapper.c
$(BINDIR)/null: $(TESTDIR)/null.c

$(BINDIR)/jk-overflow: $(TESTDIR)/overflow.c $(TESTDIR)/jk-overflow.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-underflow: $(TESTDIR)/underflow.c $(TESTDIR)/jk-underflow.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-zero: $(TESTDIR)/zero.c $(TESTDIR)/jk-zero.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-realloc: $(TESTDIR)/realloc.c $(TESTDIR)/jk-realloc.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-use-after-free: $(TESTDIR)/use-after-free.c $(TESTDIR)/jk-use-after-free.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-double-free: $(TESTDIR)/double-free.c $(TESTDIR)/jk-double-free.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-macros: $(TESTDIR)/jk-macros.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-invalid-free: $(TESTDIR)/invalid-free.c $(TESTDIR)/jk-invalid-free.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-invalid-realloc: $(TESTDIR)/invalid-realloc.c $(TESTDIR)/jk-invalid-realloc.c $(LIBDIR)/libjkmalloc.a
$(BINDIR)/jk-null: $(TESTDIR)/null.c $(TESTDIR)/jk-null.c $(LIBDIR)/libjkmalloc.a

$(TESTS):
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c

$(JKTESTS):
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c $(LIBDIR)/libjkmalloc.a

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
