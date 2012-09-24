#
# Created by gmakemake (Ubuntu Sep  7 2011) on Mon Sep 24 16:01:44 2012
#

#
# Definitions
#

.SUFFIXES:
.SUFFIXES:	.a .o .c .C .cpp .s .S
.c.o:
		$(COMPILE.c) $<
.C.o:
		$(COMPILE.cc) $<
.cpp.o:
		$(COMPILE.cc) $<
.S.s:
		$(CPP) -o $*.s $<
.s.o:
		$(COMPILE.cc) $<
.c.a:
		$(COMPILE.c) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.C.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.cpp.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%

CC =		gcc
CXX =		g++

RM = rm -f
AR = ar
LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
CPP = $(CPP) $(CPPFLAGS)
########## Flags from header.mak

ALLOCDIR =	/usr/local/pub/wrc/courses/sp1/allocate
CC =		gcc
CFLAGS =	-ggdb -std=c99 -I$(ALLOCDIR)
LIBFLAGS =	-L$(ALLOCDIR) -lallocate
CLIBFLAGS =	$(LIBFLAGS)

########## End of flags from header.mak


CPP_FILES =	
C_FILES =	project1.c
PS_FILES =	
S_FILES =	
H_FILES =	project1.h
SOURCEFILES =	$(H_FILES) $(CPP_FILES) $(C_FILES) $(S_FILES)
.PRECIOUS:	$(SOURCEFILES)
OBJFILES =	
LOCAL_LIBS =	liballocate-ubuntu86.a

#
# Main targets
#

all:	project1 

project1:	project1.o $(OBJFILES)
	$(CC) $(CFLAGS) -o project1 project1.o $(OBJFILES) $(LOCAL_LIBS) $(CLIBFLAGS)

#
# Dependencies
#

project1.o:	project1.h

#
# Housekeeping
#

Archive:	archive.tgz

archive.tgz:	$(SOURCEFILES) Makefile
	tar cf - $(SOURCEFILES) Makefile | gzip > archive.tgz

clean:
	-/bin/rm $(OBJFILES) project1.o core 2> /dev/null

realclean:        clean
	-/bin/rm -rf project1 
