#!/bin/bash
# File install.bash -- JGM -- 2022-03-11
# Version 2024-06-30  Changed ROOTDIR to /usr/local/games and re-orged the directory structure
#set -x

DISTRODIR="$PWD"
HOMEDIR="/home/${USER}"
ROOTDIR="/usr/local/games/"
#ROOTDIR="/tmp/games/"
RUNDIR="/usr/local/bin/"
#RUNDIR="/tmp/local/bin/"
PGMVER="DealerV2_4/"
PGMDIR="${ROOTDIR}${PGMVER}"
OPCDIR="${ROOTDIR}DOP/"
EXEDIR="${PGMDIR}bin/"
LIBDIR="${PGMDIR}lib/"
SUDO_USER=$USER

#copy the repo files to $PGMDIR
echo "Debug: User=${SUDO_USER} is installing from $DISTRODIR to $PGMDIR with symlinks in ${RUNDIR}"

if [[ -d ${PGMDIR} ]]; then echo Found "${PGMDIR}"
else
   echo Making "${PGMDIR}"
   mkdir -p ${PGMDIR}
fi
if [[ -d ${RUNDIR} ]]; then echo Found "${RUNDIR}"
else
   echo Making "${RUNDIR}"
   mkdir -p ${RUNDIR}
fi
cd ${DISTRODIR}
DIRLIST="bin dat Debug DebugExamples docs DOP Examples include lib Prod Regression src stdlib UserEval"
# Copy the Relevant Repo Contents to the Install Dir; preserve permission bits
 echo "Processing  realdir=${DISTRODIR} to Destdir=${PGMDIR}" 
cp -pd ${DISTRODIR}/*  ${PGMDIR}  2>/dev/null #copy the files in the main distro dir; omit subdirs for now.
 
for subdir in ${DIRLIST} ; do
    echo "Processing subdir=${subdir} realdir=${DISTRODIR}/${subdir}/" 
    cp -pRd ${DISTRODIR}/${subdir}  ${PGMDIR}
done
chown -R ${SUDO_USER}:${SUDO_USER} ${PGMDIR}

# setup the DOP/OPC Perl external program
if [[ -d ${OPCDIR} ]] ; then echo Found "$OPCDIR"
else
   echo Making "$OPCDIR"
   mkdir -p ${OPCDIR}
fi
cd  ${OPCDIR}
tar -xvf ${PGMDIR}DOP4DealerV2.tar.gz
chown -R ${SUDO_USER}:${SUDO_USER} ${OPCDIR}/*
chmod +x ${OPCDIR}dop
cd -   # back to $DISTRODIR
pwd

# now make sure that EXEDIR contains copies of all the binaries the user is likely to need
# and setup symlinks to them in the RUNDIR
echo "Setting up ${EXEDIR} and ${RUNDIR} "
for exe in Prod/dealerv2 Debug/dealdbg UserEval/DealerServer UserEval/DealerSrvdbg ; do
  progname=$( basename $exe )
  cp -p $exe ${EXEDIR}${progname}
  chmod +x   ${EXEDIR}${progname}
  ln -s      ${EXEDIR}${progname} ${RUNDIR}${progname}
done
  ln -s ${OPCDIR}/dop         ${RUNDIR}/dop
  ln -s ${LIBDIR}gibcli       ${RUNDIR}/gibcli
  ln -s ${LIBDIR}fdp          ${RUNDIR}/fdp
  ln -s ${LIBDIR}fdpi         ${RUNDIR}fdpi
  ln -s ${OPCDIR}/dop         ${EXEDIR}/dop
  ln -s ${LIBDIR}fdp          ${EXEDIR}/fdp
  ln -s ${LIBDIR}fdpi         ${EXEDIR}fdpi
# /make sure that $RUNDIR is in the users path 
append=0
if echo $PATH | grep -q "${RUNDIR}"  ; then
   echo RUNDIR found >/dev/null
else
    PATH="${RUNDIR}:${PATH}"
    append=1
fi
if [[ $append -gt 0 ]] ; then
   echo $append :: Appending[ $PATH ] to ~/.bashrc
   echo PATH=$PATH >>${HOMEDIR}/.bashrc
fi


echo "installed the OPC Perl script(s) in $OPCDIR"
echo "Complete $PGMVER set of files in $PGMDIR  including the fdpi and fdp Perl script(s) in $LIBDIR"


