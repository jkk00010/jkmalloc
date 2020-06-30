.POSIX:

LIBDIR=lib
OBJDIR=obj
SRCDIR=src
INCDIR=include
CFLAGS=-I$(INCDIR) -Wall -Wextra -Wpedantic -g

OBJECTS=$(OBJDIR)/mapalloc.o

all: $(LIBDIR)/libmapalloc.a $(LIBDIR)/libmapalloc.so

$(OBJDIR)/mapalloc.o: $(SRCDIR)/mapalloc.c $(INCDIR)/mapalloc.h
	@mkdir -p $(@D)
	$(CC) -c -o $@ $(CFLAGS) $(SRCDIR)/$(*F).c

$(LIBDIR)/libmapalloc.a: $(OBJECTS)
	@mkdir -p $(@D)
	$(AR) $(ARFLAGS) $@ $<

$(LIBDIR)/libmapalloc.so: $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) -o $@ -shared $(OBJECTS)

clean:
	$(RM) -rf $(LIBDIR) $(OBJDIR)
