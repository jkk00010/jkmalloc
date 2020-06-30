.POSIX:

BINDIR=bin
LIBDIR=lib
OBJDIR=obj
SRCDIR=src
INCDIR=include
TESTDIR=test
CFLAGS=-I$(INCDIR) -Wall -Wextra -Wpedantic -g

OBJECTS=$(OBJDIR)/mapalloc.o
TESTS=$(BINDIR)/overflow $(BINDIR)/underflow $(BINDIR)/zero

all: $(LIBDIR)/libmapalloc.a $(LIBDIR)/libmapalloc.so

tests: $(TESTS)

$(OBJDIR)/mapalloc.o: $(SRCDIR)/mapalloc.c $(INCDIR)/mapalloc.h
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/$(*F).c

$(LIBDIR)/libmapalloc.a: $(OBJECTS)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $<

$(LIBDIR)/libmapalloc.so: $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -o $@ -shared $(OBJECTS)

$(BINDIR)/overflow: $(TESTDIR)/overflow.c $(LIBDIR)/libmapalloc.a
$(BINDIR)/underflow: $(TESTDIR)/underflow.c $(LIBDIR)/libmapalloc.a
$(BINDIR)/zero: $(TESTDIR)/zero.c $(LIBDIR)/libmapalloc.a

$(TESTS):
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c -L$(LIBDIR) -lmapalloc

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
