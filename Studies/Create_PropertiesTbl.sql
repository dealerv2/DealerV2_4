create table Properties (
        DealID int primary key, 
        ClubsN tinyint unsigned not null , DiamsN   tinyint unsigned not null, HeartsN tinyint unsigned not null , 
        SpadesN tinyint unsigned not null, FreaknessN tinyint unsigned not null, LosersN tinyint unsigned not null, MltcN decimal(4,2) unsigned not null,  

        ClubsS tinyint unsigned not null , DiamsS   tinyint unsigned not null, HeartsS tinyint unsigned not null , 
        SpadesS tinyint unsigned not null, FreaknessS tinyint unsigned not null, LosersS tinyint unsigned not null, MltcS decimal(4,2) unsigned not null,  
        
        ClubsE tinyint unsigned not null , DiamsE   tinyint unsigned not null, HeartsE tinyint unsigned not null , 
        SpadesE tinyint unsigned not null, FreaknessE tinyint unsigned not null, LosersE tinyint unsigned not null, MltcE decimal(4,2) unsigned not null,  
        
        ClubsW tinyint unsigned not null , DiamsW   tinyint unsigned not null, HeartsW tinyint unsigned not null , 
        SpadesW tinyint unsigned not null, FreaknessW tinyint unsigned not null, LosersW tinyint unsigned not null, MltcW decimal(4,2) unsigned not null,  
        
)
