.POSIX:

BINDIR=bin
LIBDIR=lib
OBJDIR=obj
SRCDIR=src
INCDIR=include
TESTDIR=test
CFLAGS=-I$(INCDIR) -Wall -Wextra -Wpedantic -g -fPIC

OBJECTS=$(OBJDIR)/jkmalloc.o
WRAPOBJECTS=$(OBJECTS) $(OBJDIR)/wrap.o
TESTS=$(BINDIR)/overflow \
	$(BINDIR)/underflow \
	$(BINDIR)/zero \
	$(BINDIR)/realloc \
	$(BINDIR)/use-after-free \
	$(BINDIR)/double-free \
	$(BINDIR)/macros \
	$(BINDIR)/invalid-free \
	$(BINDIR)/invalid-realloc

all: $(LIBDIR)/libjkmalloc.a $(LIBDIR)/libjkmalloc.so

#$(LIBDIR)/libjkmalloc.so

tests: all $(TESTS) $(BINDIR)/wrapper

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
$(BINDIR)/macros: $(TESTDIR)/macros.c
$(BINDIR)/invalid-free: $(TESTDIR)/invalid-free.c
$(BINDIR)/invalid-realloc: $(TESTDIR)/invalid-realloc.c

$(BINDIR)/wrapper: $(TESTDIR)/wrapper.c
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c

$(TESTS):
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/$(@F).c -L$(LIBDIR) -ljkmalloc

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
