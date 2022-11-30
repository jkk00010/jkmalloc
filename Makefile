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
TESTS=$(BINDIR)/page-overflow \
	$(BINDIR)/page-underflow \
	$(BINDIR)/zero-alloc \
	$(BINDIR)/use-after-free \
	$(BINDIR)/double-free \
	$(BINDIR)/invalid-free \
	$(BINDIR)/invalid-realloc \
	$(BINDIR)/byte-overflow \
	$(BINDIR)/byte-underflow \
	$(BINDIR)/null

tests: all $(TESTS) $(BINDIR)/jktest

all: $(LIBDIR)/libjkmalloc.a $(LIBDIR)/libjkmalloc.so $(BINDIR)/jk

$(BINDIR)/jk: jk.sh
	@mkdir -p $(@D)
	cp -f jk.sh $@
	chmod 755 $@

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
$(BINDIR)/page-overflow: $(TESTDIR)/page-overflow.c
$(BINDIR)/page-underflow: $(TESTDIR)/page-underflow.c
$(BINDIR)/zero: $(TESTDIR)/zero.c
$(BINDIR)/use-after-free: $(TESTDIR)/use-after-free.c
$(BINDIR)/double-free: $(TESTDIR)/double-free.c
$(BINDIR)/invalid-free: $(TESTDIR)/invalid-free.c
$(BINDIR)/invalid-realloc: $(TESTDIR)/invalid-realloc.c
$(BINDIR)/null: $(TESTDIR)/null.c
$(BINDIR)/byte-overflow: $(TESTDIR)/byte-overflow.c
$(BINDIR)/byte-underflow: $(TESTDIR)/byte-underflow.c

$(BINDIR)/jktest: $(TESTDIR)/jktest.c $(LIBDIR)/libjkmalloc.a
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c $(LIBDIR)/libjkmalloc.a

$(TESTS):
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -O0 $(TESTDIR)/$(@F).c

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
