.POSIX:

BINDIR=bin
LIBDIR=lib
OBJDIR=obj
SRCDIR=src
INCDIR=include
CC=c99
CFLAGS=-I$(INCDIR) -Wall -Wextra -Wpedantic -g -fPIC
ARFLAGS=-r

OBJECTS=$(OBJDIR)/jkmalloc.o
WRAPOBJECTS=$(OBJECTS) $(OBJDIR)/wrap.o
TESTS=$(BINDIR)/jktest-dynamic $(BINDIR)/jktest-static

all: $(LIBDIR)/libjkmalloc.a $(LIBDIR)/libjkmalloc.so $(BINDIR)/jk $(TESTS)

$(BINDIR)/jk: $(SRCDIR)/jk.sh
	@mkdir -p $(@D)
	cp -f $(SRCDIR)/jk.sh $@
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

$(BINDIR)/jktest-dynamic: $(SRCDIR)/jktest.c
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) $(SRCDIR)/jktest.c

$(BINDIR)/jktest-static: $(SRCDIR)/jktest.c $(LIBDIR)/libjkmalloc.a
	@mkdir -p $(@D)
	$(CC) -o $@ $(CFLAGS) -DJK_OVERRIDE_STDLIB $(SRCDIR)/jktest.c $(LIBDIR)/libjkmalloc.a

clean:
	rm -rf $(LIBDIR) $(OBJDIR) $(BINDIR)
