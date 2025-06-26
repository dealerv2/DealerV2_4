   create table  AltC13metrics(DealID tinyint unsigned not null primary key,
       C13_NS tinyint unsigned not null, C13_BW_NS decimal(5,2) unsigned not null, C13_jgm_NS decimal(5,2) unsigned not null,C13_scaled_NS decimal(5,3) unsigned not null,
       C13_EW tinyint unsigned not null, C13_BW_EW decimal(5,2) unsigned not null, C13_jgm_EW decimal(5,2) unsigned not null, C13_scaled_EW decimal(5,3) unsigned not null )
