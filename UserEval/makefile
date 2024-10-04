# JGM Makefile for deal_v5  UserEval Server in UserEval Directory.
# This Makefile in the {ROOT} dir, i.e. {Dealer}/UserEval
#
# 2024/08/13 --  First Draft -- Rewrite of previous Makefile with each metric in its own xxxx_calc.c file
# 2024/09/17 --  Added alt_HCP_calc for various HCP metrics

CC      = gcc
CFLAGS = -std=gnu17 -mtune=corei7 -flto -fopenmp -pthread -Wall -pedantic  -I./include
#Note CPP and CPPFLAGS are for the C PreProcessor. Use CXX and CXXFLAGS for C++ aka g++
CXX = g++
CXXFLAGS = -std=gnu++17 -mtune=corei7 -flto -fopenmp -pthread  -I./include 

#can add --verbose to this next one to debug linker problems also -t (trace)
LDFLAGS =  -lm 
# LDFLAGS = -lm -t --verbose

# The dest dir is the current dir. But the Prod, Debug, and Parent dirs all have symlinks to the executable in this dir
DESTDIR = .

CALCSRCS = $(wildcard src/*calc.c )
CALCOBJS0 = $(patsubst %.c,%.o,$(CALCSRCS) ) 
CALCOBJS = $(patsubst src/%,obj/%,$(CALCOBJS0) )
MISCSRCS = src/Analyze_side.c src/metrics_util_subs.c src/Serverdebug_subs.c src/honors_calc_subs.c 
MISCOBJS0 = $(patsubst %.c,%.o,$(MISCSRCS) )
MISCOBJS = $(patsubst src/%,obj/%,$(MISCOBJS0) )

SRCS = src/UserServer.c include/UserEval_globals.c $(CALCSRCS) $(MISCSRCS) 
OBJS = obj/UserServer.o obj/UserEval_globals.o     $(CALCOBJS) $(MISCOBJS) 
HDRS  = ./include/allheaders.h ./include/dbgprt_macros.h ./include/UserEval_externs.h ./include/UserEval_types.h
HDRS += ./include/UE_calc_protos.h ./include/UE_util_protos.h ./include/std_hdrs_all.h

# This next program name is hardwired into the dealerv2 code. But can be over-ridden with the -U switch on dealer cmd line.
PROGRAM  = DealerServer

ifdef JGMDBG
CFLAGS   += -DJGMDBG  -g -Og 
CXXFLAGS += -DJGMDBG  -g -Og
PROGRAM = DealerSrvdbg
else
CFLAGS += -O2 -g
CXXFLAGS += -O2 -g
endif
ifdef SRVDATE
CFLAGS += -DSRVDATE
CXXFLAGS += -DSRVDATE
endif

${PROGRAM}: ${OBJS} $(HDRS)
	 $(CC) $(CFLAGS) -o $@ ${OBJS} $(LDFLAGS)
	 @echo Remember to copy this new version of ${PROGRAM} to the default location from which DealerV2 will call it.

showme:
	@echo BuildDate = $(SRVDATE)
	@echo DestDir = [ $(DESTDIR) ] JGMDBG = [ $(JGMDBG) ] PROGRAM = [ $(PROGRAM) ]
	@echo CC=$(CC) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS)
	@echo
	@echo HDRS=$(HDRS)
	@echo CALCSRCS=$(CALCSRCS)
	@echo CALCOBJS=$(CALCOBJS)
	@echo MISCSRCS=$(MISCSRCS)
	@echo MISCOBJS=$(MISCOBJS)
	@echo 
	@echo SRCS=$(SRCS)
	@echo OBJS=$(OBJS)
	@echo
	@echo BUILD=$(CC) $(CFLAGS) -o $(PROGRAM) ${OBJS}  $(LDFLAGS)

.PHONY : allheaders
allheaders :
	touch ./include/allheaders.h

.PHONY : allcalcs
allcalcs : $(CALCOBJS)
	@echo allcalcs done

.PHONY : allmisc
allmisc : obj/Analyze_side.o obj/metrics_util_subs.o obj/Serverdebug_subs.o obj/short_honors_subs.o obj/UserServer.o
	@echo allmisc done
	
clean:
	rm -f ${OBJS} ${LOBJ} *.gch ./include/*.gch ./include/*.o ./src/*.gch ./src/*.o a.out ./src/a.out
	
#
# File dependencies
#

$(CALCOBJS) : obj/%_calc.o : src/%_calc.c $(HDRS)
##	@echo Making Calc target obj/$*_calc.o from STEM Prereq $<
	$(CC) $(CFLAGS) -o obj/$*_calc.o -c $<

obj/Analyze_side.o :	./src/Analyze_side.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<
##	@echo Made Target $@ from Prereq $< 
	
obj/metrics_util_subs.o :  ./src/metrics_util_subs.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<
			
obj/Serverdebug_subs.o :	./src/Serverdebug_subs.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<

obj/honors_calc_subs.o :	./src/honors_calc_subs.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<
		
obj/UserEval_globals.o : include/UserEval_globals.c $(HDRS)
	@echo Making target $@ from Prereq $<
	$(CC) $(CFLAGS) -o $@ -c $<

obj/UserServer.o :  ./src/UserServer.c $(HDRS)
	$(CC) $(CFLAGS) -o $@ -c $<

