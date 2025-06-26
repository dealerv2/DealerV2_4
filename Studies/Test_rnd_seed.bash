# script to run Test_rnd_seed.dls
dealerv2 -0  10 -s  10 -v Test_rnd_seed.dls -D2 2>/tmp/seed010.txt >/tmp/seed010.dlo 
dealerv2 -0 100 -s 100 -v Test_rnd_seed.dls -D2 2>/tmp/seed100.txt >/tmp/seed100.dlo 
dealerv2 -0 200 -s 200 -v Test_rnd_seed.dls -D2 2>/tmp/seed200.txt >/tmp/seed200.dlo 
dealerv2 -0 300 -s 300 -v Test_rnd_seed.dls -D2 2>/tmp/seed300.txt >/tmp/seed300.dlo 

