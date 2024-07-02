#!/bin/bash
# File install.bash -- JGM -- 2022-03-11
# Version 2024-06-30  Changed ROOTDIR to /usr/local/games and re-orged the directory structure
set -x
DISTRODIR="$PWD"
#ROOTDIR="/usr/local/games/"
ROOTDIR="/tmp/jgm3/"
PGMVER="DealerV2_4/"
PGMDIR="${ROOTDIR}${PGMVER}"
OPCDIR="${ROOTDIR}DOP/"
if [[ ! -d ${PGMDIR} ]] ; then mkdir -p ${PGMDIR} ; fi
DIRLIST="bin dat Debug DebugExamples docs DOP Examples include lib Prod Regression src stdlib UserEval"
# Copy the Relevant Repo Contents to the Install Dir; preserve permission bits
for subdir in ${DIRLIST} ; do
    echo "Processing subdir=${subdir} realdir=${DISTRODIR}/${subdir}/" 
    cp -pRd "${DISTRODIR}/${subdir}"  "${PGMDIR}"
done
append=0
if echo $PATH | grep -q "${PGMDIR}"  ; then
   echo PGMDIR found >/dev/null
else
    PATH="${PGMDIR}:${PATH}"
    append=1
fi
if echo $PATH | grep -q "${OPCDIR}"  ; then
   echo OPCDIR found >/dev/null
else
    PATH="${OPCDIR}:${PATH}"
    append=1
fi
if [[ $append -gt 0 ]] ; then
   echo $append :: Appending[ $PATH ] to ~/.bashrc
   echo PATH=$PATH >>/tmp/bashrc 
fi
