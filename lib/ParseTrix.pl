#!/usr/bin/perl -w
# Called with: <scriptname> [-T] [-Dn] <bsmname> <file1> [.. more files ...]
# Date        Auth   Vers   Comment
# 2024/10/04  JGM     1.0   Reformat DDS tricks for import to DB
use integer;

use Getopt::Std;

@strain = ('C','D','H','S','N') ; 
$NS_file = "/tmp/NS_trix.dat";
$EW_file = "/tmp/EW_trix.dat";
#$debug_file = "DB_dbg.log";

 $Debug = 0;

 getopts('D:');
 if (defined($opt_D) ) { $Debug = $opt_D; }

print ("\nProcessing @ARGV  Debug=$Debug \n") if $Debug > 0;

open (NSFILE, ">$NS_file") || die ("Can't open $NS_file.\n");
open (EWFILE, ">$EW_file") || die ("Can't open $EW_file.\n");
# Now start crunching through the file list
while( <ARGV> ) {   # While any input in any of the files on the cmd line ....
    if (m/EOF/) { exit 0 ; }
    chomp ; 
    $line = $_; 
    print "$line\n" if $Debug > 4 ;
    $nsMax_tr = -1; $nsMax_st = -1;
    $ewMax_tr = -1; $ewMax_st = -1;
    @ns_trix = (0,0,0,0,0);
    @ew_trix = (0,0,0,0,0);
    
   ($id, @trix ) = split /,/ ;
   for ($i=0; $i<5; $i++) {
      if( $trix[$i  ] < $trix[$i+10] ) { $ns_trix[$i] = $trix[$i+10] ; } else {$ns_trix[$i] = $trix[$i   ] ; }  #pick max from N or S
      if( $trix[$i+5] < $trix[$i+15] ) { $ew_trix[$i] = $trix[$i+15] ; } else {$ew_trix[$i] = $trix[$i+15] ; }  #pick max from E or W
      if($ns_trix[$i] >= $nsMax_tr ) {$nsMax_tr = $ns_trix[$i] ;  $nsMax_st = $strain[$i] ; }  # will prefer Maj over Minor and NT over Major
      if($ew_trix[$i] >= $ewMax_tr ) {$ewMax_tr = $ew_trix[$i] ;  $ewMax_st = $strain[$i] ; }
   }
   print NSFILE "$id, $ns_trix[0], $ns_trix[1], $ns_trix[2], $ns_trix[3], $ns_trix[4], $nsMax_tr , $nsMax_st\n";  
   print EWFILE "$id, $ew_trix[0], $ew_trix[1], $ew_trix[2], $ew_trix[3], $ew_trix[4], $ewMax_tr , $ewMax_st\n";
   print  "NS_$id, $ns_trix[0], $ns_trix[1], $ns_trix[2], $ns_trix[3], $ns_trix[4], $nsMax_tr , $nsMax_st\n" if $Debug > 3;     
    
} #end while input


  print ("************ Done **********\n") if $Debug > 0;

   close NSFILE;
   close EWFILE;
__END__
 1,  7,8,7,7,8,      5,4,6,5,4,    7,9,7,7,9,  5,4,6,5,4
 2,  7,3,5,3,7,      6,10,8,10,6,  7,3,5,3,7,  6,10,8,10,6
 3,  5,6,4,9,7,      7,6,8,3,6,    5,6,4,9,7,  7,6,8,3,6
 4,  12,11,12,7,12,  0,2,0,5,0,   12,11,12,7, 12,0,2,0,5,0
 5,  7,7,4,8,6,      6,5,8,5,6,    7,7,4,8,6,  6,5,8,5,6
 6,  2,4,3,6,3,10,9,10,7,10,2,4,3,6,3,10,9,10,7,10
 7,  9,4,6,8,8,3,8,6,4,3,9,4,6,8,8,3,8,6,4,3
 8, 10,10,9,8,10,2,2,3,5,3,10,10,9,8,10,2,2,3,5,3
 9,  4,3,2,4,3,8,9,11,9,10,4,4,2,4,3,8,9,11,9,10
 10,1,1,1,5,1,10,12,12,8,9,1,1,1,5,1,10,12,12,8,9
EOF
