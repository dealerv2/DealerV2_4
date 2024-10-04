for i in $( seq 0 2 10 ) ; do echo "starting $i" ; ( /tmp/echo.bash $i & )  ;  done ; ps

/tmp/echo.bash
sleep 5 ; echo  $1

