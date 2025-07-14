#!/usr/bin/perl
# Create Test Output from the Regression tests for dls files only, using the version of dealer under test.
# Run thru all the dealer specification files that end with .dls, and generate corresponding files that end with .dlo
#     Use the same set of script vars -0 to -9 for every file.
#	   Process any .dlx files as individual exceptions at the end. e.g.swapping_s311.dlx
# This script should be in the same directory as the one with the .dls files it will be testing
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
if ( 0 == scalar(@ARGV) ) { $exe_test = "../Prod/dealerv2 " ; }
else { $exe_test = shift (@ARGV) ;  }   # should include the directory as well 

# The following are the 'standard' script vars used in the regression test. Not all used in every script 
$scriptparms = q/-0 east -1 west -2 hcp -3 top4 -4 ltc -5 12 -6 6.5 -7 spade -8 'any 4333 + any 4432 + any 5332' -9 '5+s4+mxx + 6+Mxxx'/;
# use -v to suppress end of run stats bec exec time diffs cause an err msg
$execmd = $exe_test . " -v -D0.0 " . $scriptparms ;
# print "[[ "; print  $execmd ; print " ]]\n";
$fileno = 0 ; 
@files = <*.dls> ;    # dls files that are in our current directory
foreach $input (@files) {
		# Loop over all files with suffix dls (Dealer Input sith scripting)
		print "------------------------------------------------------------------\n";
		$outfile  = $input;
		$outfile  =~ s/dls$/dlo/;
		$outfile  =~ s/..\/// ; 
		print "Now processing $input  -- creating Output/$outfile\n";
		#print "[$execmd $input >Output/$outfile] \n" ; 
		system("$execmd $input >Output/$outfile"  ); # create a test result 
		print "Created Output/$outfile \n";
		$fileno++ ;
} # end foreach input (@files)
  print "---------------Doing Individual Exception Cases (.dlx files)------------------\n";
  $input   = "swapping_s311.dlx ";
  $outfile = "swapping_s311.dlo";
  print "Now processing $input  -- creating Output/$outfile\n";
  $execmd = $exe_test . " -v -D0 -x3 " ;
  # print "[$execmd $input >Output/$outfile] \n" ; 
  system("$execmd $input >Output/$outfile"  ); 
  print "Created Output/$outfile \n";
  $fileno++ ;

print " End of Scripting Tests. $fileno files created \n" ;

