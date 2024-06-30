#!/bin/bash
REFDIR=${1:-Refer}
err=0
for of in Output/*.dlo ; do
   FN=$( basename $of .dlo )
   rf="$REFDIR/${FN}.ref";
   fileno=$(( $fileno + 1 ))
   echo "$fileno.    comparing $of to $rf"
   diff $of $rf
   if [[ $? != 0 ]] ; then err=$(( $err + 1 )) ; echo $err ; fi
done
echo There were $err files with differences out of $fileno compares


