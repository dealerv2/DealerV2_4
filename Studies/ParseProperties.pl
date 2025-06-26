#!/usr/bin/perl -w

# Date        Auth   Vers   Comment
# 2025/04  JGM     1.0   Reformat Properties for bulk load into DB

use Getopt::Std;

$O_file = "/tmp/DB_Properties.dat";

#$debug_file = "DB_dbg.log";

 $Debug = 0;

 getopts('D:');
 if ( defined($opt_D)) { $Debug = $opt_D; }

 
print ("\nProcessing @ARGV  Debug=$Debug  \n") if $Debug > 0;

open (OUTFILE, ">$O_file" ) || die ("Cant open $O_file " ) ; 


# Now start crunching through the file list
while( <ARGV> ) {   # While any input in any of the files on the cmd line ....
    chomp ; 
    $line = $_; 
    print "$line\n" if $Debug > 4 ;
	@props = split /,/ , $line;
	$ID = shift @props;
	shift @props; 
	@n    =  @props[0 .. 5] ;
	$ltcN = ($props[6] + 0.001) / 100 ;
	@s    =  @props[8 .. 13 ] ;
	$ltcS = ($props[14] + 0.001) / 100 ;
	@e    =  @props[16 .. 21 ] ;
	$ltcE = ($props[22] + 0.001) / 100 ;
	@w    =  @props[24 .. 29 ] ;
	$ltcW = ($props[30] + 0.001) / 100 ;
 
	printf OUTFILE "%d,", $ID;
	printf OUTFILE "%d ,%d ,%d ,%d ,%d ,%d, %4.2f,",(@n[0..5]) , $ltcN ; 
	printf OUTFILE "%d ,%d ,%d ,%d ,%d ,%d, %4.2f,",(@s[0..5]) , $ltcS ; 
	printf OUTFILE "%d ,%d ,%d ,%d ,%d ,%d, %4.2f,",(@e[0..5]) , $ltcE ; 
	printf OUTFILE "%d ,%d ,%d ,%d ,%d ,%d, %4.2f" ,(@w[0..5]) , $ltcW ; 
	printf OUTFILE "\n" ;    

} #end while input


  print ("************ Done **********\n") if $Debug > 0;

   close OUTFILE ;
   
__END__

