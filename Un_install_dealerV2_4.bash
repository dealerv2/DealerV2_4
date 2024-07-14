#!/bin/bash
# File UN-install.bash -- JGM -- 2024-06-30
#set -x
ROOT_ID=0 
if [[ $UID -ne $ROOT_ID ]] ; then
    echo "You should run this script as root"
    echo "Type sudo  $0 and enter password when prompted"
    exit -5
fi

echo "This will Uninstall DealerV2_4 and delete all the files in /usr/local/games/DealerV2_4 and /usr/local/games/DOP!!"
read -rp "Continue? [Yn]" ANSW
if [[ $ANSW == "Y" || $ANSW == "Yes" ]] ; then
   echo "Bombs Away !"
else
   echo "Safe choice!" ; exit 
fi

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
DIRLIST="bin dat Debug DebugExamples docs DOP Examples exe include lib Prod Regression src stdlib UserEval"
SUDO_USER=$USER

echo "Removing the symlinks in $RUNDIR "
  rm         ${RUNDIR}/dop
  rm         ${RUNDIR}/gibcli
  rm         ${RUNDIR}/fdp
  rm         ${RUNDIR}fdpi
  rm         ${RUNDIR}dealdbg
  rm         ${RUNDIR}dealerv2
  rm         ${RUNDIR}DealerServer
  rm         ${RUNDIR}DealerSrvdbg       

  
cd $ROOTDIR
chmod -R 777 ${PGMDIR}/* ${OPCDIR}/*
echo Removing "${PGMDIR} and all its files "
rm -R ${PGMDIR}/*
rmdir ${PGMDIR}

echo Removing "${OPCDIR} and all its files "
rm -R ${OPCDIR}/*
rmdir ${OPCDIR}

echo Dealver2 Version ${PGMVER} and the OPC Perl script Un-installed
echo You might want to modify your $PATH in .bashrc to remove the $RUNDIR but 
