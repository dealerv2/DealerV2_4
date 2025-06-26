#!/usr/bin/perl -w

# Date        Auth   Vers   Comment
# 2025/05/03  JGM     1.0   Reformat SB_HCP_KISS.csv for bulk load into DB

use Getopt::Std;

$O_file = "DB_HCP_KISS.dat";

#$debug_file = "DB_dbg.log";

 $Debug = 0;
 $maxlines = 11000000; # more than are in the DB
 $linecnt = 0 ; 
 getopts('D:N:O');
 if ( defined($opt_D)) { $Debug    = $opt_D; }
 if ( defined($opt_N)) { $maxlines = $opt_N; } 
 if ( defined($opt_O)) { $O_file   = $opt_O; }
 
print ("\nProcessing @ARGV  Debug=$Debug  Maxlines= $opt_N Output File= $opt_O\n") if $Debug > 0;

open (OUTFILE, ">$O_file" ) || die ("Cant open $O_file " ) ; 
$fmt = ",%5.2f" x 22 ; 
# print "$fmt\n";

 # HCP_KISSmetrics( DealID int primary key, CCCC_NS decimal(4,2) unsigned not null, ....
# Now start crunching through the file list
while( <ARGV> ) {   # While any input in any of the files on the cmd line ....
    chomp ; 
    $line = $_; 
    $linecnt++ ; 
    exit if $linecnt >= $maxlines ; 
    print "$line\n" if $Debug > 3 ;
	@fields = split /,/ , $line;
	$ID = $fields[0];
   if (scalar ( @fields ) !=   23 ) { die "Bad Record error at line $linecnt " ; }
   for ($i=1 ; $i < 23 ; $i++) {
      $pf[$i] = ($fields[$i]/100.0) + .001;
    #  printf OUTFILE "%5.2f,", $pf[$i] ;
   }
   printf OUTFILE "%d ",$ID ;
   printf OUTFILE "$fmt",@pf[1..22] ;
   print OUTFILE "\n";
   if ( $Debug > 2 ) { printf STDERR "%d ", $ID ; printf STDERR "$fmt",@pf[1..22] ; print STDERR "\n"; }

} #end while input


  print ("************ Done **********\n") if $Debug > 0;

   close OUTFILE ;
   
__END__

