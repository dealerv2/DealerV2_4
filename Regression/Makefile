# 2023/03/19 Slight Mods by JGM to use dealerv2, bash, seeds in the Descr.* files 
# 2023/03/20 Renamed all the Dealer input files from Descr.xxxxx to xxxxx.dli 
#            the Output files become xxxxx.dlo and the Refer xxxxx.ref
#            and the ones with scripting from HOW2.xxxxx to xxxxx.dls
# 2023/12/15 Added ability to test script (aka dls) files. Changed names of targets.
#
# Makefile to process all *.dli files with dealer and write the output to *.dlo
# 'make test' executes the test, 'make refer' copies the current .dlo files to .ref 
# 'make clean' cleans up, 
# Edit the line DEALER to call upon the test version or the production version.
#

DEALER = ../Prod/dealerv2    # new version of dealerv2 to be tested
#DEALER = ../Debug/dealdbg    # new debug version of dealerv2 to be tested
#DEALER = ./dealerv2         # new version to be tested copied to the Regression directory aka ../Prod/dealerv2
#DEALER = dealerv2           # reference version currently in production
SHELL=/usr/bin/bash

OUTPUT = $(wildcard ./Output/*.dlo)   # a list of all the .dlo files in the Output directory
INPUT  = $(wildcard *.dli)  $(wildcard *.dls) $(wildcard *.dlx) # a list of all the .dl? files in the Regression (current)  directory

all:
	./make_all.bash ${DEALPGM}
	
dli:
	./make_dliTests.pl $(DEALER)
	
dls:
	./make_dlsTests.pl $(DEALER)
	
# next dealer line cannot have -v (suppress eoj stats) since fmt_deal_stats needs to know how many were generated.
deal_stats:
	${DEALER}  -m chk_deal_stats_s239.dlx | ./fmt_deal_stats.pl >Output/chk_deal_stats_fmt.dlo
	if [[ -e Refer/chk_deal_stats_fmt.ref ]];then diff Output/chk_deal_stats_fmt.dlo Refer/chk_deal_stats_fmt.ref ; fi

cmp_refer:
	./cmp_dlo_to_ref.bash Refer
	
dlo_to_ref:
	./copy_dlo_to_ref.bash Refer
	
clean:
	rm -fv ${OUTPUT}
	
showme:
	@echo "Targets: all dli dls deal_stats  cmp_refer dlo_to_ref clean showme "
	@echo  
	@echo INPUT  = $(INPUT)
	@echo  
	@echo OUTPUT = $(OUTPUT)
	@echo  
	@echo DEALER = $(DEALER) Version info follows...
	$(DEALER) -V

