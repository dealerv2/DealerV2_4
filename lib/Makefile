# File Makefile in LIB directory to rebuild libdealerV2.a and libdealdbg.a
# 2023-12-10  -- attempt at a clean lib only Makefile
#
CC      = gcc $(CCFLAGS)
CCFLAGS = -std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic -O3 -I../include
LDFLAGS = -L../lib -ldealerV2 -ldds -lm 
#LDFLAGS += --verbose

LIBNAME  = libdealerV2.a
LIBSRCS  = ../src/deal_conversion_subs.c ../src/dealdebug_subs.c ../src/dealdeck_subs.c 
LIBSRCS += ../src/deal_knr.c ../src/deal_rounding_subs.c ../src/deal_scorelib.c ../src/dealutil_subs.c 
LIBOBJS = $(patsubst ../src/%.c,%.o,$(LIBSRCS) ) libVersion.o

LIBHDRS = $(patsubst ../src/%.c,%.h,$(LIBSRCS) )
LIBHDRS += ../include/dbgprt_macros.h ../include/allheaders.h

ifdef JGMDBG
CCFLAGS   += -DJGMDBG -g -O0
LIBNAME = libdealdbg.a
endif

$(LIBNAME) : $(LIBOBJS)
	ar -rcs $(LIBNAME) $?
	ar -t $(LIBNAME)
	touch -c $(LIBNAME)

update : 
	touch  $(LIBSRCS)
	make $(LIBNAME)

  # $(OBJS) : $(SRCS)   # make assumes that every object depends on every source. why the next line is better.
%.o : ../src/%.c
	$(CC) -c -o $@ $<
	
libVersion.o : ../lib/libVersion.c
		$(CC) -O0 -c -o $@ $<

clean :
	rm -f *.o a.out
		
showme:
	@echo JGMDBG = [ $(JGMDBG) ]
	@echo CC=$(CC) 
	@echo CCFLAGS = $(CCFLAGS) 
	@echo LDFLAGS = $(LDFLAGS)
	@echo LIBSRCS = $(LIBSRCS)
	@echo LIBOBJS = ${LIBOBJS}
	@echo LIBNAME=$(LIBNAME) with Members: ...
	ar -t $(LIBNAME)
	
