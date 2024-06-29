#!/bin/bash
#set -o errorexit # force an exit if unexpected error
set -o nounset   # force an exit if undefined variable Very Useful for Debugging....
#set -o verbose

#
echo "Arglist is"
echo "$@"
echo "cmd name is $0"
N=$( basename $0 )
echo "cmd base is $N"
echo "Number of args passed is: $# "
# Function Declaration using implicit form, the function keyword NOT required. Spaces around the () not significant. 
usage() {
	N=$( basename $0 ); echo "Usage: $N [-v | -h | --help ] <SourceDir> <DestDir> ";
}
usage 		# no parens in function call. to pass args say usage arg1 arg2 arg3 just like a cmd line.

TSTPGM=${1:-./dealerv2}
REFPGM=${2:-/usr/local/games/DealerV2_4/dealerv2}
export TSTPGM
export REFPGM

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
		exit -1 ;
	fi
}

err_chk
echo "err_chk passed with TSTPGM=${TSTPGM} and REFPGM=${REFPGM} "
exit 0
