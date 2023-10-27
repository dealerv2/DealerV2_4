This folder contains files that support the functioning of DealerV2 and some 
files needed for rebuilding from source.
1) libdds.a is a pre-compiled static library with Bo Haglund's Double Dummy Solver code.
The binaries of DealerV2 have been linked against this library. Since it is a static 
library its presence is not required to run DealerV2, but is required if you want to 
rebuild DealerV2

2) dds290-src_2022.tar.gz is a gzipped tar archive of the source code for the DDS library.
It is not the complete distribution of the DDS but it is sufficient to rebuild the library if needed.
Unpack it, cd to the src/ directory and type make linux to rebuild.
                                             ^^^^^^^^^^
3) DOP4DealerV2.tar is a tar archive of the Perl code that implements Optimal Point Count evaluation
The unpacked version of this file is in the DOP directory of the Repo.
To use this code from within Dealer you will need to run  sudo make install to have it copied to /usr/local/bin/DOP 
which is where Dealer expects to find it.

you can then run the dop executable in this directory with dop -h or dop -H5 for help.
Or you can browse the usage.txt file.

4) the fdp, fdpi, and Dist.pm files which contain the Perl code to process Francois Dellacherie shapes
Dealer uses fdp and Dist.pm; fdpi is an interactive version. If you run fdpi from the command line, and
then enter an FD shape specification such as:
shape{west, 55xx + 65xx - x[89]xx - xxx0 }
or
shape{west, 4+Mxxx - (4441) - 5+mxxx } 
or
shape{west, 4+M(431) - 2-mxxx - 5+mxxx:h>s }

You will see what Dealer will see when you put such a shape into your Descr Input file.
It will help you debug FD shapes.

To use this functionality you will need to do the sudo make install so that Dealer will find these files
in the /usr/local/bin/DealverV2/lib directory.

JGM
2022/03/13



