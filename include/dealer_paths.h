/* File: dealer_paths.h 2024/06/30
 * the locations for the dealerv2 executables and various helper programs, scripts and libs
 */
#ifndef DEALER_PATHS_H
#define DEALER_PATHS_H 1
#define SUDO_USER
#ifdef SUDO_USER
  #define DEAL_ROOT "/usr/local/games/"
#else
   #define DEAL_ROOT "/home/greg21/"
#endif
#define DEAL_VER_DIR "DealerV2_4/"

#define CONCAT(p1 , p2) p1 p2
#define DEAL_DIR   CONCAT(DEAL_ROOT , DEAL_VER_DIR)
#define DAT_DIR    CONCAT(DEAL_ROOT , "dat/")
#define EXE_DIR    CONCAT( DEAL_DIR , "bin/")   
#define LIB_DIR    CONCAT( DEAL_DIR , "lib/")
#define SRC_DIR    CONCAT( DEAL_DIR , "src/")
#define HDR_DIR    CONCAT( DEAL_DIR , "include/")
#define DBG_DIR    CONCAT( DEAL_DIR , "Debug")
#define PROD_DIR   CONCAT( DEAL_DIR , "Prod")
#define UEV_DIR    CONCAT( DEAL_DIR , "UserEval")
// no reason yet to define Examples, DebugExamples, Regression
// stdlib is special; it contains copies of standard Linux DLLs (.so) that might be Linux version dependant
 
#define SERVER_PGM CONCAT(EXE_DIR, "DealerServer" )
#define OPC_PGM    CONCAT(EXE_DIR, "dop" )    // ln -s DEAL_ROOT/DOP/dop
#define FDP_PGM    CONCAT(EXE_DIR, "fdp")     // ln -s LIB_DIR/fdp // In /usr/bin linux utility 'fdp' is related to graphviz
#define ZRD_LIB    CONCAT(DAT_DIR, "rpLib.zrd") // was ../rplib.zrd
/* this next one added because Linux has a BRIDGE utility that refers to ethernet cards */
/* will have to make sure there is an ln /usr/games/gibcli to /usr/games/bridge which is the real name of the GIB binary*/
#define DD_PGM "/usr/games/gibcli"
#endif  /* file guard */
