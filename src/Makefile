# JGM Makefile for dealerv2_4  2023/11/03  -- in src directory. Not used in practice.
# Note MUST use g++ as the linker stage because DDS library created with g++
# This is the src directory Makefile  Type 'make showme' to verify macro settings.
#
# 2022/10/02 -- added deal_bktfreq_subs.c to the SRCS defn
# 2022/11/15 -- added usereval_subs.c as a pre-req for dealer. Also added UserEval (the server) as a target.
# 2022/12/13 -- renamed usereval_subs.c to dealServer_subs.c; added UserEval sub-dir
# 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc.
# 2023/11/03 -- Converted some OBJ files to Library Modules for use in scaffolding and testing

CC      = gcc $(CCFLAGS)
CCFLAGS = -std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic -O3  -I../include
#Note CPP and CPPFLAGS are for the C PreProcessor. Use CXX and CXXFLAGS for C++ aka g++
CXX = g++
CXXFLAGS = -std=gnu++17 -mtune=corei7 -flto -fopenmp -pthread  -O3  -I../include
#can add --verbose to this next one to debug linker problems
LDFLAGS = -L../lib -lm -ldds -ldealerV2
DESTDIR = ../src

ifdef JGMDBG
CCFLAGS   += -DJGMDBG -g -O0
CXXFLAGS += -DJGMDBG -g -O0
DESTDIR = ../src
endif

FLEX    = flex
YACC    = bison -d

PROGRAM  = dealjgm

LIBNAME  = libdealerV2.a
LIBBASE  = dealerV2
LIBSRCS  = ../src/deal_conversion_subs.c ../src/dealdebug_subs.c ../src/dealdeck_subs.c 
LIBSRCS += ../src/deal_knr.c ../src/deal_scorelib.c ../src/dealutil_subs.c ../lib/libVersion.c
LIBOBJS = $(patsubst ../src/%.c,%.o,$(LIBSRCS) )
LIBHDRS = $(patsubst ../src/%.c,%.h,$(LIBSRCS) )
LIBHDRS += ../include/dbgprt_macros.h ../include/allheaders.h

SRCS  = ../src/dealaction_subs.c ../src/deal_bias_subs.c ../src/deal_bktfreq_subs.c  ../src/dealcard_subs.c 
SRCS += ../src/dealdds_subs.c    ../src/Dealer_DDS_IF.c  ../src/dealeval_subs.c   
SRCS += ../src/dealinit_subs.c ../src/dealparse_subs.c   ../src/dealrpdd_subs.c ../src/dealServer_subs.c
SRCS += ../src/dealerv2.c 
OBJS = $(patsubst ../src/%.c,%.o,$(SRCS) ) dealglobals.o dealflex.o dealyacc.tab.o              


HDR  = ../include/allheaders.h ../include/dbgprt_macros.h ../include/libdealerV2.h
# Need to link with g++ because libdds library was built with c++
dealsrc: ${OBJS} ../lib/$(LIBNAME) $(HDR)
	@echo Making dealsrc with target $@ LDFLAGS=$(LDFLAGS) and ... 
	@echo OBJS=$(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ ${OBJS}  $(LDFLAGS)
# no rule to make the library in this Makefile; see ../lib/Makefile
allheaders :
	touch ../include/allheaders.h

clean:
	rm -f ${OBJS} ${LOBJ} ${YOBJ} ${LIBOBJS} *.gch ../include/*.gch ../include/*.o

showme:
	@echo DestDir = $(DESTDIR) JGMDBG = $(JGMDBG)
	@echo CC=$(CC) $(CCFLAGS) $(LDFLAGS)
	@echo CXX=$(CXX) $(CXXFLAGS) $(LDFLAGS)
	@echo BISON SRC= $(YSRC) OBJ= $(YOBJ)
	@echo FLEX  SRC= $(LSRC) OBJ=$(LOBJ)
	@echo
	@echo HDR=$(HDR)   LIBBASE=$(LIBBASE) LIBRARY=$(LIBNAME)
	@echo SRCS=$(SRCS)
	@echo
	@echo OBJS=$(OBJS)
	@echo
	@echo BUILD=$(CXX) CXXFLAGS -o dealdbg ${OBJS}  $(LDFLAGS)
	@echo HDR=$(HDR)   LIBBASE=$(LIBBASE) LIBRARY=$(LIBNAME)
	@echo LIBSRCS=$(LIBSRCS) 
	@echo LIBOBJS=$(LIBOBJS)


#
# Flex
#
LSRC = ../src/dealflex.l dealyacc.tab.h
LOBJ = dealflex.c

$(LOBJ) : $(LSRC) $(HDR)
	${FLEX} -o $@ $<

#
# Yacc/Bison -- bison -d creates the file dealyacc.tab.c and dealyacc.tab.h
#

YSRC = ../src/dealyacc.y
YOBJ = dealyacc.tab.c dealyacc.tab.h

$(YOBJ) : $(YSRC) $(HDR)
	${YACC} $<
#
# File dependencies
#

dealerv2.o : ../src/dealerv2.c ../src/mainsubs.c ../src/UsageMsg.h $(HDR)
	$(CC)  -o $@ -c $<
dealflex.o : dealflex.c $(HDR)
	$(CC)  -o $@ -c $<
dealyacc.tab.o : dealyacc.tab.c $(HDR)
	$(CC)  -o $@ -c $<
deal%.o : ../src/deal%.c $(HDR)
	$(CC)  -o $@ -c $<
Dealer_DDS_IF.o : ../src/Dealer_DDS_IF.c $(HDR)
	$(CC)  -o $@ -c $<
dealglobals.o : ../include/dealglobals.c $(HDR)
	$(CC)  -o $@ -c $<
dealServer_subs.o : ../src/dealServer_subs.c ../include/mmap_template.h $(HDR)
	$(CC)  -o $@ -c $<
mainsubs.o : mainsubs.c
	@echo Not Making mainsubs.o


objects : $(OBJS)

user_eval.o : ../src/user_eval.c  ../include/mmap_template.h
	$(CC)  -o $@  -c $<
UserEval : user_eval.o
	$(CC)  -o $@  $<
#
# Library
#

