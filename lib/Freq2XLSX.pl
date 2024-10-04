#!/usr/bin/perl -w

###################################################################
# Takes input from Dealerv2 Freq2D and BktFreq2D actions output; produces Excel Spreadsheet
#
# Called with: <scriptname> -Dn -f <.xlsx Workbook Name>  <infiles list>    
#
#             -Dn Debug switch. Higher numbers are more verbose and include all ouput from lower numbers.    
#                 0 No Debug Output
#                 1,2 Echoes filenames, prints Done message.
#------------------------------------------------------------------
# Date        Auth   Vers   Comment
# 2024/06/27  JGM     1.0   So I can cumulative graph tricks vs Metrics using Excel
# 2024/07/21  JGM     2.0   Added formulas to the sheet. Reversed the order of tricks High down to Low. Incl labels
# 2024/07/21  JGM     2.1   Changed Tag numbers for HCP flavor metrics
#
###################################################################
use integer;

use Getopt::Std;
use Excel::Writer::XLSX;
use Excel::Writer::XLSX::Utility;
 
# ( $row, $col ) = xl_cell_to_rowcol( 'C2' );    # (1, 2)
# $str           = xl_rowcol_to_cell( 1, 2 );    # C2
#
 our $workbook ; 
 our ( $worksheet, $sheetName, $hdgfmt, $valfmt, $pctform, $pctfmt )  ;
 our ($InTable, $vrow, $next_rownmu, $Hi_rownum, $Lo_rownum ) ;
 our (%d_flds, $mID, $mName , $strain) ;
 our @colHdgs ;
 our ($tag, @vals) ;
 our ($des, $id_t, $id_v, $nm_l, $nm_v, $st_l, $st_v, $sz_l, $sz_v, $se_l, $se_v, $comment) ;
 our $Debug = 0 ;

 our %MetricNames = 
                 (0 => BERGEN, 1 => BISSEL, 2 => DKP, 3 => GOREN, 4 => KARP_B,
                  5 => KAPLAN, 6 => KARPIN, 7 => KnR, 8 => LAR,    9 => LAR_B,
                 10 => PAV, 11 => SHEINW, 12 => ZARBAS, 13 => ZARADV, 14 => ROTH, 15 => OPC,
                 # some DealerServer metrics for alternate ways of evaluating HCP only. No D, no Dfit, or Length etc.
                 20 => "HCP_TEN++", 21 => "HCP_ACE+", 22 => "HCP_A+T++",
                 23 => Bumwrap,     24 => Woolsey,    25 => Andrews_5ths,
                 26 => BWjgm,       27 => OPCjgm ,
                 # some HCP metrics built into Dealer; no Server Required 
                 30 => WorkHCP,     31 => AcesC13,        32 => CCCC  );
                 
                 
 # short names for spreadsheet tabs                
 our %MetricIDs =(0 => BERG,  1 => BISS,    2 => DKP,     3 => GOR,     4 => KARB,
                  5 => KAP,   6 => KARP,    7 => KnR,     8 => LAR,     9 => LARB,
                 10 => PAV,  11 => SHEN,   12 => ZAR,    13 => "ZAR+", 14 => ROTH, 15 => OPC, # end of both hands with fit.
                 # some DealerServer metrics for alternate ways of evaluating HCP only. No D, no Dfit, or Length etc.
                 20 => TEN050,  21 => ACE425, 22 => AT475,   
                 24 => BUMW, 25 => WOOL,   25 => A5TH,   26 => BWjgm, 27 => OPCjgm,
                 # some HCP metrics built into Dealer; no Server Required                 
                 30 => HCP, 31 => C13, 32 => CCCC  );
              
 our @cols = ( 'A' .. 'Z' ) ;  
sub numerically { $a <=> $b }

sub show_arr  {         # passed an array descr as a scalar string and an array_ref
        my $a_name = shift @_;
        my $idx = 0 ;
        my $a_ref = shift @_ ;
        my ($item, $s_item) ;
        # printf STDERR "a_ref points to: %s \n", ref($a_ref);
        # printf "STDERR Array[0] = %s \n", $a_ref->[0];
        printf STDERR "<------------Array %12s has %6d items --------------->\n", $a_name, scalar(@{$a_ref});
        foreach $item ( @{$a_ref} ) {
                $s_item = sprintf "%10.30s", $item;
                printf STDERR "\t %d = %-20.30s \n", $idx, $s_item ;
                $idx++;
        }  # end foreach item
}  #end sub show_arr

sub TitleLine () {
   # Ex. Description: ID=14 Name=ROTH Strain=NoTrump Size=5000000 Seed=5000 DDS Tricks in NoTrump vs Roth Points
   $_[0] =~ s/^\s+// ; 
   ($des, $id_t, $id_v, $nm_l, $nm_v, $st_l, $st_v, $sz_l, $sz_v, $se_l, $se_v, $comment) = split /[= ]/ , $_[0], 12 ;
   my @dflds = ($id_t, $id_v, $nm_l, $nm_v, $st_l, $st_v, $sz_l, $sz_v, $se_l, $se_v ) ;
   show_arr("In Split dflds", \@dflds) if $Debug >= 2 ; 
   return @dflds ; # ID 15 Name OPC Strain NoTrump Size 500000 Seed 5000
                   # $comment available as a global for later use
}

sub HdgLine () {
   $_[0] =~ s/^\s+// ; 
   my @hdg = split /\s+/ , $_[0]; # Ex. Low  7  8  9   10   11   12  High  Sum
   show_arr("In Split hdg", \@hdg) if $Debug >= 4 ;
   $hdg[7] = " 13" ;  
   @hdg = ( reverse (@hdg[0 .. 7] ), $hdg[8] ); # want the tricks to be from High to low since shows up better on charts.
   show_arr("Reversed Hdg Labels", \@hdg) if $Debug >= 3 ;
   return @hdg ; 
} # end sub HdgLine

sub LowLine ($) {
   $_[0] =~ s/^\s+// ; 
   my @LowVals = split /\s+/ , $_[0];  # Ex: 2050 Low 7  8  9  10  11  12  13  Sum 
   show_arr("In Split LowVals", \@LowVals) if $Debug >= 4 ; 
   @LowVals =  ( $LowVals[0], reverse (@LowVals[1 .. 8]) , $LowVals[9] );
   show_arr("Reversed LowVals", \@LowVals) if $Debug >= 3 ;
   return @LowVals ; 
} # end sub LowLine
#
sub MidLine ($) {
   $_[0] =~ s/^\s+// ; 
   my @MidVals = split /\s+/ , $_[0];
   show_arr("In Split MidVals", \@MidVals) if $Debug >= 4 ;
   @MidVals = ( $MidVals[0], reverse (@MidVals[1 .. 8]) , $MidVals[9] );
   show_arr("Reversed MidVals", \@MidVals) if $Debug >= 3 ;
   return @MidVals ;
} # end sub MidLine
#
sub HiLine ($) {
   $_[0] =~ s/^\s+// ; 
   my @HiVals = split /\s+/ , $_[0];
   show_arr("In Split HighVals", \@HiVals) if $Debug >= 4 ;
   @HiVals = ( $HiVals[0], reverse (@HiVals[1 .. 8] ) , $HiVals[9] );
   show_arr("Reversed HiVals", \@HiVals) if $Debug >= 3 ;
   return  @HiVals;
} # end sub HiLine

sub TotLine ($) {
   $_[0] =~ s/^\s+// ; 
   my @Totals = split /\s+/ , $_[0];
   show_arr("In Split Totals", \@Totals) if $Debug >= 4 ;
   @Totals = ( $Totals[0], reverse (@Totals[1 .. 8]) , $Totals[9] );
   show_arr("Reversed Totals", \@Totals) if $Debug >= 3 ;
   
   return  @Totals ;
} # end sub TotLine
#
sub EndTable() {
   print STDERR "Finished with worksheet $sheetName \n" ;
   print STDERR "Inserting Formulas Now \n" ;
   # &fill_formulas($Lo_rownum, 2, $Hi_rownum ) ; First Data in col B2 (13 tricks )
   $Lo_rownum = 0 ;
   $Hi_rownum = 0 ;
   $vrow = 1 ;
   $next_rownum = 0 ;
   %d_flds = ( ) ;   # undef all the fields.
}


sub New_sheet {
   my $dflds_ref = shift ;
      show_hash(" New_Sheet d_flds", $dflds_ref) if $Debug > 2;
      my $mID = $dflds_ref->{Tag} ;
      my $strain = $dflds_ref->{Strain} ; 
      $sheetName = $mID . "_" . $MetricIDs{ $mID } . "_" . $strain ;
      $worksheet = $workbook->add_worksheet($sheetName);
      print STDERR "Sheetname: $sheetName : created \n" if $Debug >= 1 ;
      # print STDERR $mID, $strain, $MetricIDs{ $mID }, "**\n" if $Debug > 0 ; 
      $worksheet->set_row(0, undef, $hdgfmt);  # row number, height, format, ....
      $worksheet->set_row(1, undef, $hdgfmt);
      $worksheet->set_column(0, 0, undef , $hdgfmt); #start col, end col, format, ....
      print STDERR "Worksheet set_row and set_col fmts done\n" if $Debug >= 1 ;

      $worksheet->write('A1', "Metric ID");
      $worksheet->write('B1', $mID);         # Numeric value
      $worksheet->write('C1', "Strain");
      $worksheet->write('D1', $strain);
      $worksheet->write('E1', "Size");
      $worksheet->write('F1', $dflds_ref->{Size});
      $worksheet->write('G1', "Seed");
      $worksheet->write('H1', $dflds_ref->{Seed});
      print STDERR "Wrote title fields\n" if $Debug >= 2 ; 
      return $worksheet ;
}

sub show_hash {         # passed in a hash descr/name, hashref, prints out sorted keys and values
       my $h_name = shift @_;
       my $h_ref  = shift @_;
       my ($h_key, $ref_t);
       $ref_t = ref $h_ref;
       printf STDERR "<--------Hash %12s of type %10s has %6s buckets  --------->\n",
               $h_name, $ref_t, scalar(%{$h_ref});
        foreach $h_key (sort keys %{$h_ref} ) {
           if ( defined ($h_ref->{$h_key})) {
              printf STDERR "%30s  => %30s\n", $h_key, $h_ref->{$h_key} };
        } # end foreach key
} # end sub show_hash
 
# begin main
 print "In Main. Calling getopts\n";
 getopts('D:f:');
 print "Cmdline Opts: D=$opt_D f=$opt_f\n";
 if (defined($opt_D) ) { $Debug = $opt_D; }
 die "Must provide a Filename for the Spreadsheet Output file" if (!defined($opt_f) ) ;

 my  $datline ; 
 print "Calling new workbook on $opt_f \n" ;
# Create a new Excel workbook
$workbook =  Excel::Writer::XLSX->new($opt_f);
die "Creating new Excel $opt_f FAILED. $!" unless defined $workbook ;   
$hdgfmt = $workbook->add_format(); # Add a format Set properties later
$hdgfmt->set_bold();
# $format->set_color('red');
$hdgfmt->set_align('center');
$valfmt  = $workbook->add_format(); #
$valfmt->set_align('center');
$pctfmt = $workbook->add_format() ;
$pctfmt->set_num_format('0.00%');
 
print ("\nProcessing @ARGV\nDebug=$Debug Outfile=$opt_f \n") if $Debug > 0;

# &show_hash( "Metric Names", \%MetricIDs ) if $Debug > 2;
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ begin Main Loop ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$inTable = 0 ;
$vrow = 1 ;      
$Hi_rownum = 0 ; # perl numbering. Excel Row 0 does not exist and will cause errors.
$Lo_rownum = 0 ; # perl numbering.
$next_rownum = 0 ; 

# Now start crunching through the file list
while( <> ) {   # While any input in any of the files on the cmd line ....
    $datline = $_; chomp($datline);
    $_ = $datline ;
    print "Input:: $datline :: \n" if $Debug >= 2 ; 
    if ( $datline =~ /.*Description:(.*)/) {
      &EndTable() if $inTable ; # Prob not much to do to close off previous sheet
      %d_flds = &TitleLine($datline) ; # convert a list to a hash
      show_hash("Main:: d_flds from TitleLine", \%d_flds) if $Debug >= 2 ;
      $worksheet = &New_sheet (\%d_flds ) ;
      $next_rownum = 1 ;   # vrow = 1  perl numbering; row zero is the sheet title and description.
     } #end Description aka Title

     elsif ( $datline =~ /^\s*Low.*Sum\s*$/ ) {
         # Found Header Line
         @colHdgs = &HdgLine( $datline ) ;
         $worksheet->write_row(1,1, \@colHdgs, $hdgfmt);  # B2 Row 2 Col B
         print STDERR "Wrote Header fields\n" if $Debug >= 3 ;
         $next_rownum = 2 ;    #vrow = 1
     } # end header
      elsif ( $datline =~ /^\s*Low/ && $datline !~ /Sum\s*$/) {
         # Found LowVals Line
         ($tag, @vals) = &LowLine( $datline ) ;
         $worksheet->write(2, 0, $tag, $hdgfmt ) ;       # A3 row 3 col A
         $worksheet->write_row(2,1, \@vals, $valfmt);    # B3 rThe tricks values in cols B .. I and Sum in J 
         $Lo_rownum   = 2 ;
         $next_rownum = 3 ;   #vrow = 1
         print STDERR "Wrote Low Values Row 3 \n" if $Debug >= 3 ;
     } # end Low Vals
      elsif ($datline =~   /^\s*High/ && $datline !~ /Sum\s*$/) {
         # Found HiVals Line
         ($tag, @vals) = &HiLine( $datline ) ;
         $worksheet->write(2+$vrow, 0, $tag, $hdgfmt ) ;
         $worksheet->write_row(2+$vrow ,1, \@vals, $valfmt); # Ex. 4075 13 12 11 10 9 8 7 Low Sum
         $next_rownum++ ;
         $Hi_rownum = 2 + $vrow ;
         $vrow++ ;   
         print STDERR "Wrote High Values Row (2 + $vrow) Hi_rownum = $Hi_rownum \n" if $Debug >= 3 ;
     } # end High Vals
     elsif ( $datline =~ /^\s*Sum.*/ ) {
         # Found Totals Line
         ($tag, @vals) = &TotLine( $datline ) ;
         $worksheet->write(2+$vrow, 0, $tag, $hdgfmt ) ;
         $worksheet->write_row(2+$vrow ,1, \@vals, $valfmt);
         $next_rownum++ ;   #next_rownum should now be the last row used in the Excel sheet (Excel numbering)
         print STDERR "Wrote Totals Values Row (2 + $vrow) \n" if $Debug > 2 ;
     } # end Totals
     elsif ( $datline =~ /\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/  ) {
         # Found MidVals Line  --
         # first time thru vrow = 1; so write goes to row 3 (Perl Numbering) Descr=0, Hdg=1, Low=2
         ($tag, @vals) = &MidLine( $datline ) ;
         $worksheet->write(2+$vrow, 0, $tag, $hdgfmt ) ; # The 'Points' value in Col A row .
         $worksheet->write_row(2+$vrow ,1, \@vals, $valfmt); # The tricks values in cols B .. I and Sum in J 
         print STDERR "Wrote Data Values Row (2 + $vrow) \n" if $Debug >= 3 ;
         $vrow++ ;
         $next_rownum++ ;  
     }

   } # end while
   # Data all put. Now create formulas for the Percentages in Cols L .. R
   # Don't do pcts for the 'low' i.e. 6 tricks and less numbers since will just clutter charts.
   print STDERR "Calling Make_chart with High Row= $Hi_rownum\n" if $Debug > 2 ;
   my @tricks = @colHdgs[0 .. 6];
   $worksheet->write('N1','Percentages');
   $worksheet->write_row('L2',\@tricks );  # give names to the cols for the chart
   &Fill_formulas( 2 , 1 , $Hi_rownum ) ;
   print STDERR "Fill Formulas returns \n" if $Debug >= 3 ;
   # $workbook->close();
   # exit 0 ; 

   print STDERR "Calling Make_chart with High Row= $Hi_rownum\n" if $Debug > 2 ;
   my $stacked_chart = &Make_chart( $worksheet, "Stacked Chart", 3, $Hi_rownum ) ;
     print STDERR "Make_chart returns \n" if $Debug > 2 ; 
   $workbook->close();
   exit 0 ; 
# end main
sub Fill_formulas  {   #worksheet, start_row, start_col, high_row 
  my $start_row = shift ;   #typically the First Cell with data will be B3 so row=2, and col=1
  my $start_col = shift ;
  my $high_row  = shift ;   # the last row with valid data in it for this freq table
  my ($rownum, $rowname);
  my ($colnum, $colname, $dstcolnum, $dstcolname, $src_cell, $sum_cell, $cellname);
  my $offset = 10 ;  # 'L' - 'B'
  # my $formula = $worksheet->store_formula( '=B3/$J3');
  # my $rowlabel= $worksheet->store_formula( '=A3' );

  print "Filling Formulas: Startcol=$cols[ $start_col ] From Row=$start_row,  to Row $high_row\n";
# Write 7 columns of formulas based on the stored formulas
   for $rownum ($start_row .. $high_row) { # using perl (NOT EXCEL) numbering
      $rowname = $rownum + 1;   # Excel row numbers start at 1; Perl row numbers start at zero
      $dst_cell = xl_rowcol_to_cell( $rownum, 10 );
      $src_cell = xl_rowcol_to_cell( $rownum,  0 );
      $worksheet->write_formula($dst_cell,  "=$src_cell", $hdgfmt  ) ; # copy An to Kn
      print STDERR "Fill_formula wrote =$src_cell to $dst_cell\n" if $Debug >= 3;
      for $colnum ( 1 .. 7 ) {  # Perl col 1 is Excel Col 'B'; col A contains a label for the row
         $src_cell = xl_rowcol_to_cell( $rownum, $colnum );    # B3 start
         $sum_cell = 'J' . $rowname ;
         $dstcolnum = ($colnum + $offset) ;
         $dst_cell = xl_rowcol_to_cell( $rownum, $dstcolnum );
         $worksheet->write_formula($rownum, $dstcolnum, "=$src_cell/$sum_cell", $pctfmt);
         # print STDERR "\tFill_formula wrote =A$src_cell/$sum_cell to cell $dst_cell \n") if $Debug >= 3;
         print ("\tFormula in Cell at (r,c)=$rownum, $colnum, with src_cell=$src_cell and sum_cell=$sum_cell DestCell=$dst_cell \n") if $Debug >= 3;
      } # end for colnum
   } # end for $rownum
   return ; 
} # end sub fill_formulas   

# workbook is a global ...
# Data Source Sheet Excel Numbering
# Row 1 : Title, Metric ID, Strain etc.
# Row 2 : Name of series, i.e. number of tricks Cols L thru R (11-17)
# Row 3 : Category Low K3;(row 2, col 10) Data cols L thru R. Ignore this row
# Row 4 : Category Start Col K(row 3 col 10); Values Col L thru R
# Row n : Category High. Col K; Values Col L thru R
sub Make_chart {
   my $worksheet = shift ;  # source of data
   my $chartsheet = shift;  # name of sheet to contain the chart; if it is simply '1' then we embed the chart
   my $row_start = shift ;   # should always be '2' i.e. Excel Row 3
   my $row_end   = shift ;   # col start is always 'L' aka 11 and col end is always R aka 17
   my $chart ;
   my $embed = 0 ; 
   if ( $chartsheet eq "1" ) {
      $chartsheet = $worksheet . "Chart" ;
      $embed = 1 ; 
      $chart = $workbook->add_chart( name => $chartsheet, type => 'bar' , subtype => 'stacked', embedded => 1 );
   }
   else {
       $chart = $workbook->add_chart( name => $chartsheet, type => 'bar' , subtype => 'stacked' );
   }

   print STDERR "Make Chart with Name= $chartsheet, row_start=$row_start, row_end=$row_end\n" if $Debug >= 3;

   for my $col ( 11 .. 17) {  # Cols L thru R  11 17
      # Note use of array ref to define ranges: [ $Worksheet, $row_start, $row_end, $col_start, $col_end ].
      $chart->add_series(
         name       => [ $sheetName, 1, $col ],  # L2, M2, N2, etc.
         categories => [ $sheetName, $row_start, $row_end, 10, 10 ], #K3 thru Knn
         values     => [ $sheetName, $row_start, $row_end, $col,$col ], #L3 thru Lnn, M3 thru Mnn etc
      );
      print STDERR "Make Chart did series for Col=$col row_start=$row_start row_end=$row_end\n" if $Debug >= 3;
   } # end for $col
      $workbook->close();
   exit 0 ; 
     
# Add some labels.
   $chart->set_title( name => 'Percentages of Tricks Taken' );
   $chart->set_x_axis( name => 'Percents' );
   $chart->set_y_axis( name => 'Side Evaluation' );
  print STDERR "Make Chart Labels Added x_axis = Percents \n" if $Debug >= 3;
   if ($embed == 1 ) {
      # Insert the chart into the worksheet (with an offset).
      $worksheet->insert_chart( $row_end+4, 2, $chart, { x_offset => 25, y_offset => 10 } );
   }
   # else chart sheet with chart already created? in line 317?
   return $chart ;
}  # end make chart






   
__END__ 
perl -e '$arr[0]=1; $arr[1]=2; $arr[2]=-6 ; print "$arr[0], $arr[2], $#arr, ", scalar(@arr), "\n";' => 2, -6, 2, 3
perl -e 'my @arr = (2,4,-6); print "$arr[0], $arr[2], $#arr, ", scalar(@arr), "\n";' => 2, -6, 2, 3

=pod
MetricID   Name
--------------------------- UserEval Metrics. Usually Values + Dpts(short) + Lpts + Dfit pts + Short suit corrections
   0     BERGEN            // from internet pdfs
   1     BISSEL,           // from Pavlicek's web site
   2     DKP,              // Danny Kleinman from The NoTrump Zone
   3     GOREN,            // from Pavlicek's web site
   4     JGM1,             //
   5     KAPLAN,           // from his book
   6     KARPIN,           // from Pavlicek's web site
   7     KnR,              // from the Bridge World Article as coded by JGM including Dfit pts not found in cccc metric
   8     LAR,              // Larsson from his book, Good Better Best
   9     MORSE,            // Larsson with BumWrap pts and some Dfit mods
  10     PAV,              // Pavlicek from his web site. Almost the same as Goren
  11     SHEINW,           // from his book
  12     ZARBAS,           // Zar points; basic version. From online PDFs
  13     ZARADV,           // Zar points; Full version with all corrections and add ons.
  14     ROTH,             // from the Roth Reubens book circa 1960
  15     OPC               // Optimal Point Count from P. Darricades books and correspondance
  20 => HCP,               // 4.0 3.0 2.0  1.0  0.0      Sum = 10 
  21 => C13_Aces,          // 6.0 4.0 2.0  1.0  0.0      Sum = 13
  22 => CCCC,              // KnR Calculator Required
  23 => Bumwrap,           // 4.5 3.0 1.5  0.75 0.25    Sum = 10
  24 => Woolsey,           // 4.5 3.0 1.75 0.75 0.0     Sum = 10

  21 => "HCP_TEN++",
  22 => "HCP_ACE+",
  23 => "HCP_A+T++",
  24 => Bumwrap,           // 4.5 3.0 1.5  0.75 0.25    Sum = 10

  26 => Andrews_5ths,      // 4.0 2.8 1.8  1.0  0.4     Sum = 10
  
  

  -------------------------- Various Flavors of HCP. No Dpts, Lpts, Dfit Pts, Deductions etc. Total Two Hands. ---------
altcount 9 6.0 4.0 2.0 1.0         // C13 Burnstein in Dotnum scale           sum=13
altcount 6 4.0 2.8 1.8 1.0 0.4     // Andrews Fifths                          sum=10
altcount 5 4.5 3.0 1.5 0.75 0.25   // Bergen aka C13 * 3/4                    sum=10
altcount 4 4.5 3.0 1.75 0.75       // Woolsey HCP                             sum=10
altcount 3 4.5 3.0 1.5 0.75 0.25   // BumWrap                                 sum=10
altcount 2 4.25 3.0 2.0 1.0 0.5    // Work with A=4.25 and T=0.5 Per Andrews  sum=10.75
altcount 1 4.25 3.0 2.0 1.0        // Work with A=4.25           Per Andrews  sum=10.25
altcount 0 4.0 3.0 2.0 1.0 0.5     // Work with T=0.5                         sum=10.5
pointcount 4.0 3.0 2.0 1.0         // Work std in DecNum for same scale       sum=10 

    // title "Pavlicek's Spec for hands that might play in NT "
condition  
 shape(south,any xxxx - any xxx7 - any xxx0 - any xxx1 - 6xxx - x6xx) &&
 shape(north,any xxxx - any xxx7 - any xxx0 - any xxx1 - 6xxx - x6xx) &&
   hcp(south)+hcp(north) >= 10.0

// Description: Tag 22
action

   bktfreq "Description: Tag 20 Name HCP Strain NoTrump :Top=35.00 step=1.0"       (hcp(south)+hcp(north),15.0,35.0,1.0,  dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 21 Name C13  Strain NoTrump  :Top=343.00 step=1.0"    (pt9(south)+pt9(north),15.0,43.0,1.0,dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 22 Name CCCC Strain NoTrump  :Top=35.00 step=0.5"     (cccc(south)+cccc(north),15.0,35.0,0.5,dds(south,notrump),7,12,1),   
   bktfreq "Description: Tag 23 Name CPU_Bergen Strain NoTrump :Top=35. step=0.25" (pt5(south)+pt5(north),15.0,35.0,0.25, dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 24 Name Woolsey  Strain NoTrump :Top=35.00 step=0.25" (pt4(south)+pt4(north),15.0,35.0,0.25, dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 25 Name Bumwrap  Strain NoTrump :Top=35.00 step=0.25" (pt3(south)+pt3(north),15.0,35.0,0.25, dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 26 Name Fifths  Strain NoTrump :Top=335.00 step=0.2"  (pt6(south)+pt6(north),15.0,35.0,0.2,dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 27 Name A+ Strain NoTrump :Top=35.00 step=0.25"       (pt1(south)+pt1(north),15.0,35.0,0.25, dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 28 Name Ten++ Strain NoTrump :Top=35.00 step=0.5"     (pt0(south)+pt0(north),15.0,35.0,0.5,  dds(south,notrump),7,12,1),
   bktfreq "Description: Tag 29 Name A+_Ten++ Strain NoTrump :Top=35.00 step=0.25" (pt2(south)+pt2(north),15.0,35.0,0.25, dds(south,notrump),7,12,1),
   
 
 --------------------------------
 Sample Output Pts Vs Tricks: (some rows snipped; rows Low and High may be missing. Col Hdr always Present


Description: HCP in Dotnum    
           Low        7        8        9       10       11       12     High    Sum
 Low         0        0        0        0        0        0        0        0        0
1000     48077        0        0        0        0        0        0        0    48077
1100     74615        3        0        0        0        0        0        0    74618
1200    106963       11        3        0        0        0        0        0   106977
1300    148676       80        0        1        0        0        0        0   148757
1400    196797      397       34        4        0        0        0        0   197232
1500    245349     1597      254       22        0        0        0        0   247222
1600    293053     5177      828       75        7        0        0        0   299140
1700    327962    14122     2645      310       26        2        0        0   345067
1800    341696    33023     7598     1017       90        6        0        0   383430
1900    323777    63208    18624     2972      356       32        0        0   408969
2000    269070    99140    39081     7874     1115      123       11        1   416415
2100    193849   122131    68617    18604     3236      428       33        3   406901
2200    121839   119070    96781    35755     8023     1221      174       23   382886
2300     67224    93284   107135    57128    17003     3281      502       87   345644
2400     34148    59537    93807    73012    30352     7073     1211      211   299351
2500     16451    32464    65313    73233    43944    13595     2744      495   248239
2600      7940    16352    36779    55889    51472    21776     5320     1054   196582
2700      3624     7750    17614    33107    47021    29192     9126     1777   149211
2800      1735     3644     7509    14943    32578    31479    13131     2849   107868
2900       732     1710     2957     5220    16903    26721    16145     3822    74210
3000       318      707     1115     1455     6319    17335    16515     4769    48533
3100       111      255      330      312     1518     8380    13594     5358    29858
3200        35       86      107       43      313     2824     9132     4786    17326
3300        13       19       17        6       31      615     4653     4087     9441
3400         0        2        2        0        6       85     1800     2815     4710
3500         1        0        0        0        0        4      500     1565     2070
High         0        0        0        0        0        1       87     1178     1266
Sum    2824055   673769   567150   380982   260313   164173    94678    34880  5000000

All known HCP scales in DotNum format vs Tricks in NT
Generated 5064926 hands
Produced  5000000 hands
Library file records read 5064926, Wrap Arounds=0
Library file starting seed 5000
Time needed   12.906 sec

=cut
