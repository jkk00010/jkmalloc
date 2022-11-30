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
TESTS=$(BINDIR)/jktest-dynamic $(BINDIR)/jktest-static

all: $(LIBDIR)/libjkmalloc.a $(LIBDIR)/libjkmalloc.so $(BINDIR)/jk $(TESTS)

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

$(BINDIR)/jktest-dynamic: $(TESTDIR)/jktest.c
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(TESTDIR)/jktest.c

$(BINDIR)/jktest-static: $(TESTDIR)/jktest.c $(LIBDIR)/libjkmalloc.a
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -DJK_OVERRIDE_STDLIB $(TESTDIR)/jktest.c $(LIBDIR)/libjkmalloc.a

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
