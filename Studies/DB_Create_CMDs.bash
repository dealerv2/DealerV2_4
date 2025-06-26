cd /home/greg21/DealerV2_4/Studies
../Prod/dealerv2 -v -L ../dat/rpLib.zrd -C /tmp/TricksNS.tbl DB_Tricks.dli >/tmp/TricksEW.tbl <=== post process via perl to get max strain and max tricks
../Prod/dealerv2 -v -L ../dat/rpLib.zrd -C /tmp/Cards.tbl DB_Cards.dli
../Prod/dealerv2 -v -L ../dat/rpLib.zrd -U ../Prod/DealerServer  -C /tmp/DB_NS_AltHCP.tbl DB_AltHCP.dli >/tmp/DB_EW_AltHCP.tbl <=== post process via perl to change RAW into real
../Prod/dealerv2 -v -L ../dat/rpLib.zrd -U ../Prod/DealerServer  -C /tmp/DB_NS_Metrics.tbl DB_Metrics.dli >/tmp/DB_EW_Metrics.tbl <=== post process via perl to change RAW into real

