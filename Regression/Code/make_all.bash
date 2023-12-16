#!/bin/bash
TSTPGM=${1:-./dealerv2}
REFPGM=${2:-/usr/local/bin/DealerV2/dealerv2}
export DEALPGM
export REFPGM

function usage() {
	echo "basename($0) [ [tst_dealer] production_dealer] e.g. $0 ../Debug/dealdbg /usr/local/DealerV2/dealerv2 " 
}

function err_chk() { 
 	errcnt=0 
	if [[ !(-f ./make_dliTests.pl) ]] ; then
		echo "ER** Cannot find ./make_dliTests.pl in current directory. "
		errcnt=$(( $errcnt + 1 ))
	fi
	if [[ !(-f ./make_dlsTests.pl) ]] ; then
		echo "ERR** Cannot find ./make_dlsTests.pl in current directory. "
		errcnt=$(( $errcnt + 1 ))
	fi
	if [[ !(-f ./make_dliTests.pl) ]] ; then
		echo "ERR** Cannot find ./make_dliTests.pl in current directory. "
		errcnt=$(( $errcnt + 1 ))
	fi
	if [[ !(-f ./fmt_deal_stats.pl) ]] ; then
		echo "ERR** Cannot find ./fmt_deal_stats.pl in current directory. "
		errcnt=$(( $errcnt + 1 ))
	fi
	if [[ !(-d Output) ]] ; then
		echo "ERR** Sub-directory Output does not exist. "
		errcnt=$(( $errcnt + 1 ))
	fi

	if [[ !(-d Refer) ]] ; then
		echo "ERR** Sub-directory Refer does not exist. "
		errcnt=$(( $errcnt + 1 ))
	fi

	if [[ !(-x ${REFPGM} ) ]] ; then
		echo "ERR** Reference exectutable ${REFPGM} does not exist "
		errcnt=$(( $errcnt + 1 ))
	fi
	
	if [[ !(-x ${TSTPGM} ) ]] ; then
		echo "ERR** Test exectutable ${TSTPGM} does not exist "
		errcnt=$(( $errcnt + 1 ))
	fi	
   if [[ $errcnt > 0 ]] ; then
		echo " $errcnt missing files. Cannot proceed. Correct and rerun."
		usage  
		exit -1 
	fi
}

err_chk 		#check that we are in the correct directory and that the necessary scripts and sub-Dirs are there

echo "Re Making the .ref files in the Refer directory using ${REFPGM}"
./make_dliTests.pl $REFPGM
./make_dlsTests.pl $REFPGM
${REFPGM}  -m chk_deal_stats_s239.dlx | ./fmt_deal_stats.pl >Refer/chk_deal_stats_fmt.ref 
make dlo_to_ref 

echo "Running make <dli dls deal stats> using ${TSTPGM}"
./make_dliTests.pl ${TSTPGM} 
./make_dlsTests.pl ${TSTPGM}
echo "Processing chk_deal_stats_s239.dlx to Output/chk_deal_stats_fmt.dlo "
${TSTPGM}  -m chk_deal_stats_s239.dlx | ./fmt_deal_stats.pl >Output/chk_deal_stats_fmt.dlo 

echo "Comparing .dlo files in Output to corresponding .ref file in Refer"
./cmp_dlo_to_ref.bash Refer

echo "- - - - - - -  Done ! - - - - -  "

