This is the Notes subdirectory of the dealerv2 Regression directory. The Regression  directory and its subdirectories contain:
1. Perl scripts to automate the running of the regression tests. See below.
2. A Bash script to compare all the xxxx.dlo files to the equivalent xxxx.ref file using the diff command and report errors.
   The script is not terribly robust as even a mild difference between the two files (such as the time taken) would be reported as an error.
3. A Makefile that will automatically run the correct scripts for a given target.
   Valid targets are:  all dli dls deal_stats  cmp_refer dlo_to_ref clean showme 
4. Files ending with ".dli", ".dls". ".dlt", ".dlx" These are input files to dealerv2 that are meant to test the dealerv2 functionality.
   The scripts will automatically run dealerv2 using each one of these files as input.
5. A subdirectory, Output, with files ending with ".dlo" These files (if present) are the result of running dealer with the corresponding .dli 
   input file. The script will automatically redirect the dealer output which normally goes to the screen to one of these files.
6. A subdirectory, Refer, with files ending with ".ref" These files (if present) are the output of a version of dealerv2 known to work. 
   If dealerv2 is rebuilt or modified, then re-running the regression test should result in output files that differ only 
   slightly (if at all) from the xxxxx.ref files.
7. File "chk_deal_stats.dli". This file is also a dealerv2 input file, that is a separate target in the makefile.
   use the command make deal_stats
9. File "eval.dat". To use the GIB functionality of dealerv2, this file MUST be in the CURRENT WORKING directory, in this
   case 'Regression'. You must have a copy of this file in every directory from which you intend to call the GIB functionality 
   of dealerv2. If you do not call the GIB functionality, but use the DDS functionality instead you do not need this file.
--------------------------------------------------------------------------------------------------------------------------
The PERL scripts:
make_dliTests.pl : This script will loop over all the files that end with ".dli' or '.dlt' and run dealerv2 on them.
      The output is sent to a file named "xxxxx.dlo" in the Output subdirectory, where the filename prefix is the same as the one 
      that was on the .dli|.dlt file.  The end of run stats are suppressed for files ending in .dli, but are kept for files ending in .dlt (timing)

make_dlsTests.pl : This script will loop over all the files that end with ".dls' and run dealerv2 on them using fixed set of command line parms.
      The output is sent to a file named "xxxxx.dlo" in the Output subdirectory, where the filename prefix is the same as the one 
      that was on the .dls file. The script will also process any files ending in .dlx as special exception files that don't fit into the above.
     
fmt_deal_stats.pl this script will take the output of the from running dealerv2 on the chk_deal_stats.dli file 
                  and reformat it to make it easy to compare to the theoretical figures.
--------------------------------------------------------------------------------------------------------------------------
The BASH scripts:

copy_dlo_to_ref.bash :  This shell script will make a copy of the .out files that can be used for reference. Replaces .dlo suffix with .ref 
                   >>>> Pass the name of the Refer directory on the cmd line E.G. ./copy_out_to_ref.bash ./Refer
                        Can be used to create the first copy of .ref, or to save the results of a previous regression run to the Refer directory.
cmp_dlo_to_ref.bash  :  This will run the 'diff' utility on the .dlo and .ref files and report any differences.
                   >>>> Pass the name of the Refer directory on the cmd line E.G. ./cmp_out_to_ref.bash ./Refer 
make_all.bash        : This shell script takes two optional parameters: the pathname of the version of dealer under test, 
                        such as ../Debug/dealdbg, (default is ./dealerv2 in the current directory) and 
                        the pathname of the production version of dealer known to produce good output, such as
                        /usr/local/bin/DealerV2/dealerv2 (which is the default value).
                        
                        The script first overwrites all the .dlo files in Output by running the the 3 perl scripts above, 
                        using the production version of dealerv2, and then
                        runs the copy_dlo_to_ref.bash script to make copies of the new .dlo files in the Refer subdirectory with new 
                        suffixes, .ref
                        
                        Secondly it overwrites all the .dlo files in Output by running the the 3 perl scripts above, 
                        using the test version of dealerv2.
                        
                        Lastly it runs the cmp_dlo_to_ref.bash script to compare the test version of the outputs, to the production version. 

-----------------------------------------------------------------------------------------




