#!/usr/bin/perl -w

# Date        Auth   Vers   Comment
# 2024/10/29  JGM     1.0   Reformat Metrics for bulk load into DB

use Getopt::Std;

$NS_file = "/tmp/DB_NS_Metrics.dat";
$EW_file = "/tmp/DB_EW_Metrics.dat";
#$debug_file = "DB_dbg.log";

 $Debug = 0;

 getopts('D:S:');
 if ( defined($opt_D)) { $Debug = $opt_D; }
 if (!defined($opt_S)) { die "Must provide a Side: -SN, -SE, -SS, -SW" ; }
 else { $side = $opt_S ; }
 
print ("\nProcessing @ARGV  Debug=$Debug Side=$side \n") if $Debug > 0;

if(     $side eq 'N' or $side eq 'S' ) { open (OUTFILE, ">$NS_file" ) || die ("Cant open $NS_file " ) ; }
elsif ( $side eq 'E' or $side eq 'W' ) { open (OUTFILE, ">$EW_file" ) || die ("Cant open $EW_file " ) ; }
else { die "Invalid Side spec $side "; }

# Now start crunching through the file list
while( <ARGV> ) {   # While any input in any of the files on the cmd line ....
    chomp ; 
    $line = $_; 
    print "$line\n" if $Debug > 4 ;
    
   ($id, @metrics ) = split /,/ ;
   $m_hldf_NT_str = join ',', @metrics[0..31] ; 
   # @m_hldf_NT = @metrics[0..31] ;
   
   @m_raw  = @metrics[32..43] ; #HLDF Karp_B, KnR, Lar_B, ZarB, ZarA, OPC  then NT Karp_B, KnR, Lar_B, ZarB, ZarA, OPC
   printf OUTFILE  ("%d,%s,", $id,$m_hldf_NT_str) ;
   printf OUTFILE  ("%.2f,%.2f,%.2f,%d,%d,%.2f",(0.001+($m_raw[0]/100.0)),(0.001+($m_raw[1]/100.0)),(0.001+($m_raw[2]/100.0)),$m_raw[3],$m_raw[4],(0.001+($m_raw[5]/100.0)) );
   printf OUTFILE  ("%.2f,%.2f,%.2f,%d,%d,%.2f\n",(0.001+($m_raw[6]/100.0)),(0.001+($m_raw[7]/100.0)),(0.001+($m_raw[8]/100.0)),$m_raw[9],$m_raw[10],(0.001+($m_raw[11]/100.0)));
   if ($Debug > 3 ) {
      printf  ("%d,%s,", $id,$m_hldf_NT_str) ;
      printf  ("%.2f,%.2f,%.2f,%d,%d,%.2f",(0.001+($m_raw[0]/100.0)),(0.001+($m_raw[1]/100.0)),(0.001+($m_raw[2]/100.0)),$m_raw[3],$m_raw[4],(0.001+($m_raw[5]/100.0)) );
      printf  ("%.2f,%.2f,%.2f,%d,%d,%.2f\n",(0.001+($m_raw[6]/100.0)),(0.001+($m_raw[7]/100.0)),(0.001+($m_raw[8]/100.0)),$m_raw[9],$m_raw[10],(0.001+($m_raw[11]/100.0)));
   }
} #end while input


  print ("************ Done **********\n") if $Debug > 0;

   close OUTFILE ;
   
__END__

