#!/usr/bin/perl -w

# Date        Auth   Vers   Comment
# 2025/04  JGM     1.0   Reformat Properties for bulk load into DB

use Getopt::Std;

$O_file = "/tmp/DB_C13.dat";

#$debug_file = "DB_dbg.log";

 $Debug = 0;
 $maxlines = 11000000; # more than are in the DB
 $linecnt = 0 ; 
 getopts('D:N:');
 if ( defined($opt_D)) { $Debug = $opt_D; }
 if ( defined($opt_N)) { $maxlines = $opt_N; } 

 
print ("\nProcessing @ARGV  Debug=$Debug  \n") if $Debug > 0;

open (OUTFILE, ">$O_file" ) || die ("Cant open $O_file " ) ; 

 # AltC13metrics(DealID, C13_NS, C13_BW_NS, C13_jgm_NS, C13_scaled_NS,C13_EW, C13_BW_EW, C13_jgm_EW, C13_scaled_EW )
# Now start crunching through the file list
while( <ARGV> ) {   # While any input in any of the files on the cmd line ....
    chomp ; 
    $line = $_; 
    $linecnt++ ; 
    exit if $linecnt >= $maxlines ; 
    print "$line\n" if $Debug > 3 ;
	@fields = split /,/ , $line;
	$ID = $fields[0];
   # NS fields
   $fields[2] = ($fields[2]+0.001) / 100.0 ;   # C13_BW
   $fields[3] = ($fields[3]+0.001) / 100.0 ;    # C13_jgm
   $c13_scaled_ns = ($fields[1] * 10.0 / 13.0) + 0.0001 ; # 3 decimal places for this one.
   # EW fields
   $fields[5] = ($fields[5]+0.001) / 100.0 ;   # C13_BW
   $fields[6] = ($fields[6]+0.001) / 100.0 ;   # C13_jgm
   $c13_scaled_ew = ($fields[4] * 10.0 / 13.0) + 0.0001 ;    
                                                           
	printf OUTFILE "%d,%d,%4.2f,%4.2f,%5.3f,  %d,%4.2f,%4.2f,%5.3f\n", 
            #         NS: C13      C13_BW     C13_jgm                  EW: C13      C13_BW     C13_jgm
                  $ID,$fields[1],$fields[2],$fields[3], $c13_scaled_ns, $fields[4], $fields[5], $fields[6], $c13_scaled_ew  ; 

  if ( $Debug > 2 ) {
	printf STDERR "%d,%d,%4.2f,%4.2f,%5.3f,  %d,%4.2f,%4.2f,%5.3f\n", 
            #         NS: C13      C13_BW     C13_jgm                  EW: C13      C13_BW     C13_jgm
                  $ID,$fields[1],$fields[2],$fields[3], $c13_scaled_ns, $fields[4], $fields[4], $fields[4], $c13_scaled_ew  ; 
  }
    

} #end while input


  print ("************ Done **********\n") if $Debug > 0;

   close OUTFILE ;
   
__END__

