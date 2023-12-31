# JGM Makefile for deal_v5  2022/12/13  -- in src directory. Not used in practice.
# Note MUST use g++ as the linker stage because DDS library created with g++
# This is the src directory Makefile  Type 'make showme' to verify macro settings.
#
# 2022/10/02 -- added deal_bktfreq_subs.c to the SRCS defn
# 2022/11/15 -- added usereval_subs.c as a pre-req for dealer. Also added UserEval (the server) as a target.
# 2022/12/13 -- renamed usereval_subs.c to dealServer_subs.c; added UserEval sub-dir
# 2023/01/07 -- Merged in changes from V4 to fix predeal; dealcards_subs.c and globals, etc.

CC      = gcc
CFLAGS = -std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic -O3 -g -Og -I../include
#Note CPP and CPPFLAGS are for the C PreProcessor. Use CXX and CXXFLAGS for C++ aka g++
CXX = g++
CXXFLAGS = -std=gnu++17 -mtune=corei7 -flto -fopenmp -pthread  -O3 -g -Og -I../include
#can add --verbose to this next one to debug linker problems
LDFLAGS = -L../lib -lm -ldds
DESTDIR = ../src

ifdef JGMDBG
CFLAGS   += -DJGMDBG
CXXFLAGS += -DJGMDBG
DESTDIR = ../Debug
endif

FLEX    = flex
YACC    = bison -d

PROGRAM  = dealjgm

SRCS  = ../src/dealaction_subs.c ../src/dealdds_subs.c ../src/dealdebug_subs.c ../src/dealeval_subs.c ../src/dealcard_subs.c
SRCS += ../src/dealparse_subs.c ../src/Dealer_DDS_IF.c ../src/c4.c ../src/deal_bktfreq_subs.c
SRCS += ../src/dealServer_subs.c ../src/dealerv2.c

OBJS = $(patsubst ../src/%.c,%.o,$(SRCS) ) dealglobals.o dealflex.o dealyacc.tab.o

HDR  = ../include/allheaders.h ../include/dbgprt_macros.h
# Need to link with g++ because libdds library was built with c++
dealsrc: ${OBJS}
	$(CXX) $(CXXFLAGS) -o $@ ${OBJS}  $(LDFLAGS)

allheaders :
	touch ../include/allheaders.h

clean:
	rm -f ${OBJS} ${LOBJ} ${YOBJ} *.gch ../include/*.gch

showme:
	@echo DestDir = $(DESTDIR) JGMDBG = $(JGMDBG)
	@echo CC=$(CC) $(CFLAGS) $(LDFLAGS)
	@echo CXX=$(CXX) $(CXXFLAGS) $(LDFLAGS)
	@echo BISON SRC= $(YSRC) OBJ= $(YOBJ)
	@echo FLEX  SRC= $(LSRC) OBJ=$(LOBJ)
	@echo
	@echo HDR=$(HDR)
	@echo SRCS=$(SRCS)
	@echo
	@echo OBJS=$(OBJS)
	@echo
	@echo BUILD=$(CXX) CXXFLAGS -o dealdbg ${OBJS}  $(LDFLAGS)


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

dealerv2.o : ../src/dealerv2.c ../src/mainsubs.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
dealflex.o : dealflex.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
dealyacc.tab.o : dealyacc.tab.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
deal%.o : ../src/deal%.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
Dealer_DDS_IF.o : ../src/Dealer_DDS_IF.c $(HDR)
	$(CC) $(CFLAGS) -o $@ -c $<
c4.o : ../src/c4.c ../include/c4.h $(HDR)
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
