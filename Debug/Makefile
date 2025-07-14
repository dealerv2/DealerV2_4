# JGM Makefile for dealerV4 2023/11/03 DEBUG directory
# Note MUST use g++ as the linker stage because DDS library created with g++
# This is the Debug directory Makefile so set the JGMDBG variable. Type 'make showme' to verify its effect.
#
# 2022/10/02 -- added deal_bktfreq_subs.c to the SRCS defn
# 2022/11/12 -- added usereval_subs.c as a pre-req for dealer. Also added UserEval (the server) as a target.
# 2022/12/13 -- renamed usereval_subs.c to dealServer_subs.c; added UserEval sub-dir
# 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc.
# 2023/03/14 -- Added dealrpdd_subs and dealinit_subs to requirements
# 2023/11/03 -- Major re-Org to allow some pieces of DealerV2 to be library modules.
# 2023/11/30 -- Changed name of library to libdealdbg.a to be able to debug library code with GDB
# 2024/01/14 -- Added dealzrd_subs.c dealzrd_defs.h removed dealrpdd_subs.c 

JGMDBG := 1
CC      = gcc
CFLAGS = -std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic -g -O2 -I../include 
#Note CPP and CPPFLAGS are for the C PreProcessor. Use CXX and CXXFLAGS for C++ aka g++
CXX = g++
CXXFLAGS = -std=gnu++17 -mtune=corei7 -flto -fopenmp -pthread  -g -O2 -I../include 
#can add --verbose to this next one to debug linker problems; order matters. linker pulls in un resolved symbols in left objs from right libs.
LDFLAGS = -L../lib  -l$(LIBBASE)  -ldds -lm
DESTDIR = ../Prod
PROGRAM  = dealerv2

LIBDIR   = ../lib
LIBNAME  = libdealerv2.a
LIBBASE  = dealerV2
LIBSRCS  = ../src/dealdebug_subs.c dealdeck_subs.c ../src/deal_knr.c deal_scorelib.c dealutil_subs.c 
LIBHDRS = $(patsubst ../src/%.c,%.h,$(LIBSRCS) )
LIBHDRS += ../include/dbgprt_macros.h ../include/allheaders.h

ifdef JGMDBG
CFLAGS   += -DJGMDBG -O0 -g
CXXFLAGS += -DJGMDBG -O0 -g
DESTDIR = ../Debug
PROGRAM  = dealdbg
LIBNAME = libdealdbg.a
LIBBASE  = dealdbg
endif

FLEX    = flex
BISON_FLAGS = -Wcounterexamples
YACC    = bison -d 

SRCS  = ../src/dealaction_subs.c ../src/deal_bias_subs.c ../src/deal_bktfreq_subs.c  ../src/dealcard_subs.c 
SRCS += ../src/dealdds_subs.c    ../src/Dealer_DDS_IF.c  ../src/dealeval_subs.c   
SRCS += ../src/dealinit_subs.c ../src/dealparse_subs.c   ../src/dealServer_subs.c
SRCS += ../src/dealzrd_subs.c ../src/dealerv2.c 
OBJS = $(patsubst ../src/%.c,%.o,$(SRCS) ) dealglobals.o dealflex.o dealyacc.tab.o              

HDR  = ../include/allheaders.h ../include/dbgprt_macros.h ../include/libdealerV2.h 
# Need to link with g++ because libdds library was built with c++

dealdbg: ${OBJS} ${HDR} $(LIBDIR)/$(LIBNAME)
	$(CXX) $(CXXFLAGS) -o $@ ${OBJS}  $(LDFLAGS)

allheaders :
	touch ../include/allheaders.h

clean:
	rm -f ${OBJS} ${LOBJ} ${YOBJ} *.gch ../include/*.gch

showme:
	@echo DestDir = $(DESTDIR) JGMDBG = [ $(JGMDBG) ] LIBBASE=$(LIBBASE)
	@echo 
	@echo CC=$(CC) $(CFLAGS) $(LDFLAGS)
	@echo CXX=$(CXX) $(CXXFLAGS) $(LDFLAGS)
	@echo BISON SRC= $(YSRC) OBJ= $(YOBJ)
	@echo FLEX  SRC= $(LSRC) OBJ=$(LOBJ)
	@echo
	@echo HDR=$(HDR)   LIBBASE=$(LIBBASE) LIBRARY=$(LIBNAME)
	@echo SRCS=$(SRCS)
	@echo
	@echo OBJS=$(OBJS)
	@echo
	@echo LDFLAGS=$(LDFLAGS)
	@echo BUILD=$(CXX) $(CXXFLAGS) -o dealdbg ${OBJS} $(LDFLAGS)
	@echo LIBRARY=$(LIBNAME) LIBHDRS=$(LIBHDRS)
	@echo LIBSRCS=$(LIBSRCS) 
	@echo LIBHDRS=$(LIBHDRS)

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

YSRC = ../src/dealyacc.y ../include/dbgprt_macros.h
YOBJ = dealyacc.tab.c dealyacc.tab.h

$(YOBJ) : $(YSRC) $(HDR)
	${YACC} $<
#
# File dependencies
#

dealerv2.o : ../src/dealerv2.c ../src/mainsubs.c ../src/UsageMsg.h $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
dealflex.o : dealflex.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
dealyacc.tab.o : dealyacc.tab.c $(HDR)
deal%.o : ../src/deal%.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
Dealer_DDS_IF.o : ../src/Dealer_DDS_IF.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
dealglobals.o : ../include/dealglobals.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
dealServer_subs.o : ../src/dealServer_subs.c ../include/mmap_template.h $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<

objects : $(OBJS)

user_eval.o : ../src/user_eval.c  ../include/mmap_template.h
	$(CC) $(CFLAGS) -o $@  -c $<
UserEval : user_eval.o
	$(CC) $(CFLAGS) -o $@  $<
#
# Library
#
# No rule to make the library in this makefile; see ../lib/Makefile 
$(LIBNAME) : $(LIBSRCS) ../lib/libVersion.c
	@echo The Library is out of date. See the Makefile in the ../lib directory
