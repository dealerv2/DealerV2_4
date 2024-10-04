Directory stdlib
The files in this directory are standard library files that came with the version of 
Linux used to build this Repo.

The filenames ending in ".a' are static libraries that the user can use if he
is trying to build his own copy of dealerv2 from source, and does not have the
dynamic versions available.
They were not used to build the binary distributed with this Repo.

Not all of the '.so' files have a '.a' version available.

The files with ".so" in their names are 'shared object' files; i.e. dynamic
linked libraries that are added to dealerv2 by the linux kernel at runtime.
It has been known to happen that some versions of linux have different versions
of these files or name these files differently.
The complete pathnames of these libraries can be found by running the command:
ldd Prod/dealerv2

If the dealerv2 binary will not run on your system, because of a missing 
library try copying the corresponding ".so" file to the 'correct' location and
see if that works. You will need to be super-user (or use sudo) to do this.

The 'linux-vdso.so.1' "library" is not a real library. It is part of the 
Linux kernel itself. You should not need to do anything about this object.

............................ sample ldd dealerv2 output ---------------
$ ldd /usr/local/bin/dealerv2 
	linux-vdso.so.1 (0x00007fffbcfe1000)
	libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007fb140ecc000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007fb140de5000)
	libgomp.so.1 => /lib/x86_64-linux-gnu/libgomp.so.1 (0x00007fb140d9b000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007fb140d7b000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fb140b52000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fb14135b000)

