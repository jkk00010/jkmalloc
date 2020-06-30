.POSIX:

BINDIR=bin
LIBDIR=lib
OBJDIR=obj
SRCDIR=src
INCDIR=include
TESTDIR=test
CFLAGS=-I$(INCDIR) -Wall -Wextra -Wpedantic -g -fPIC

OBJECTS=$(OBJDIR)/mapalloc.o
WRAPOBJECTS=$(OBJECTS) $(OBJDIR)/wrap.o
TESTS=$(BINDIR)/overflow \
	$(BINDIR)/underflow \
	$(BINDIR)/zero \
	$(BINDIR)/realloc \
	$(BINDIR)/use-after-free \
	$(BINDIR)/double-free \
	$(BINDIR)/macros

all: $(LIBDIR)/libmapalloc.a $(LIBDIR)/libwrapalloc.so

#$(LIBDIR)/libmapalloc.so

tests: $(TESTS) $(BINDIR)/wrapper

$(OBJDIR)/mapalloc.o: $(SRCDIR)/mapalloc.c $(INCDIR)/mapalloc.h
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/$(*F).c

$(OBJDIR)/wrap.o: $(SRCDIR)/wrap.c $(INCDIR)/mapalloc.h
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/$(*F).c

$(LIBDIR)/libmapalloc.a: $(OBJECTS)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $<

$(LIBDIR)/libmapalloc.so: $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -o $@ -shared $(OBJECTS)

$(LIBDIR)/libwrapalloc.so: $(WRAPOBJECTS)
	@mkdir -p $(@D)
	$(CC) -o $@ -shared $(WRAPOBJECTS)

$(TESTS): $(LIBDIR)/libmapalloc.a
$(BINDIR)/overflow: $(TESTDIR)/overflow.c
$(BINDIR)/underflow: $(TESTDIR)/underflow.c
$(BINDIR)/zero: $(TESTDIR)/zero.c
$(BINDIR)/realloc: $(TESTDIR)/realloc.c
$(BINDIR)/use-after-free: $(TESTDIR)/use-after-free.c
$(BINDIR)/double-free: $(TESTDIR)/double-free.c
$(BINDIR)/macros: $(TESTDIR)/macros.c

$(BINDIR)/wrapper: $(TESTDIR)/wrapper.c
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c

$(TESTS):
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c -L$(LIBDIR) -lmapalloc

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
