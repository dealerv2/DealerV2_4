#!/bin/bash
REFDIR=${1:-Refer}
err=0
for of in Output/*.dlo ; do
   FN=$( basename $of .dlo )
   rf="$REFDIR/${FN}.ref";
   fileno=$(( $fileno + 1 ))
   echo "$fileno.    copy $of to $rf"
   cp $of $rf
 done
   echo Copied $fileno .dlo files to $REFDIR .ref files


