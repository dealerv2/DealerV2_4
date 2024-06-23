#define UsageMessage "List of Run Time Options (all optional): [hmqvVg:p:s:x:C:D:L:M:O:P:R:T:N:E:S:W:X:U:Z:l:0:1:2:3:4:5:6:7:8:9:]\n" \
"h=Help      m={progress Meter}     q={PBN Quiet mode} \n"  \
"v={Verbose, toggle EOJ stats}      V={show Version info and exit}\n" \
"\nThese next switches all require values either integers or strings \n" \
"g={override Inputfile Generate}                \np={override Inputfile Produce}\n" \
"s={override Inputfile starting Seed for RNG} \nx={eXchangeMode:2|3}\n" \
"C={Filename for CSV Report. (Precede with w: to truncate, else opened for append)}\n" \
"N|E|S|W={Compass predeal holding - override input predeal statement}\n" \
"O={OPC evaluation Opener(N|S|E|W) Default=[W|S]}\n" \
"L={Use Path as RP Library source. Default Path = ../rpdd.zrd \n"\
"M={dds_Mode: 1=single solution, 2=20x solutions}  \nR={Resources/Threads(1..9)}\n" \
"P={vulnerability for Par computation: 0=NoneVul, 1=NS, 2=EW, 3=Both}\n"  \
"T={Title in quotes}    \n" \
"U={DealerServer pathname - program name or relative or absolute pathname}\n" \
"X={Filename to open for eXporting predeal holdings}\n" \
"Z={-Z [Nw:]<Fname> Append deals and 20 DD solutions in RP zrd fmt. N:no DDS, w:truncate }\n" \
"l={-l [Nw:]<Fname> Append deals and 20 DD solutions in DL52 fmt.   N:no DDS, w:truncate }\n" \
"D={Debug verbosity level 0-9[.0-9]; (minimal effect in production version)}\n" \
"-0 to -9={set $0 thru $9 script parms in Inputfile one word or many in quotes}\n"
