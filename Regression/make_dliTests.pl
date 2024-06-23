#!/usr/bin/perl
# Create Test Output from the Regression tests only, using the version of dealer under test.
# Run thru all the dealer specification files that end with .dli or .dlt and generate corresponding files that end with .dlo
# This script should be in the same directory of the one with the .dli files it will be testing
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
if ( 0 == scalar(@ARGV) ) { $exe_test = "../Prod/dealerv2 " ; }
else { $exe_test = shift (@ARGV) ;  }   # should include the directory as well 
$fileno = 0 ; 
@files = <*.dli> ;    # dli files are in our current directory
foreach $input (@files) {
  # Loop over all files with suffix dli (Dealer Input)
  print "------------------------------------------------------------------\n";
  $outfile  = $input;
  $outfile  =~ s/dli$/dlo/;
  $outfile  =~ s/..\/// ; 
  print "Now processing $input  -- creating Output/$outfile\n";
  system("$exe_test -v -D0.0 $input >Output/$outfile"  ); # create a test result # use -v to suppress end of run stats.

  print "Created Output/$outfile \n";
  $fileno++ ;

} # end foreach input (@files)

print " End of dli Tests. $fileno files created \n" ;
  print "---------------- Doing Some Timing tests .dlt files -------------------------\n";
@files = <*.dlt> ;    # dlt files are in our current directory
foreach $input (@files) {
  print "------------------------------------------------------------------\n";
  $outfile  = $input;
  $outfile  =~ s/dlt$/dlo/;
  $outfile  =~ s/..\/// ; 
  print "Now processing $input  -- creating Output/$outfile\n";
  $execmd = $exe_test . " -D2.0 -m " ; #generate eoj stats for timing and for counts of calls  
  # print "[$execmd $input >Output/$outfile] \n" ; 
  system("$execmd $input >Output/$outfile"  ); 
  print "Created Output/$outfile \n";
  $fileno++ ;
}
print " End of dli and dlt Tests. $fileno files created \n" ;
