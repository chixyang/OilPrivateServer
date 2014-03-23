create database OilPrivateDB default character set 'utf8' collate 'utf8_general_ci';

use OilPrivateDB;

create table UserDetail
(
	username varchar(20) not null primary key,      
	password varchar(20), 							
	cellphone int unsigned,     					
	real_name varchar(15)                  			
)engine=InnoDB default charset=utf8;


create table LevelFirstInfo
(
	time bigint unsigned not null,        
	drum tinyint unsigned not null,      
	level smallint unsigned not null,    
	primary key (time,drum)              
)engine=InnoDB default charset=utf8;


create table LevelSecondInfo
(
	time bigint unsigned not null,		  
	drum tinyint unsigned not null,       
	avg_level smallint unsigned not null, 
	num tinyint unsigned not null,		  
	primary key (time,drum)
)engine=InnoDB default charset=utf8;



create table LevelThirdInfo
(
	time bigint unsigned not null,		 
	drum tinyint unsigned not null,      
	avg_level smallint unsigned not null,	 
	num smallint unsigned not null,		  
	primary key (time,drum)
)engine=InnoDB default charset=utf8;



create table TempFirstInfo
(
	time bigint unsigned not null,       
	drum tinyint unsigned not null,      
	temp float not null,    			
	primary key (time,drum)              
)engine=InnoDB default charset=utf8;


create table TempSecondInfo
(
	time bigint unsigned not null,		  
	drum tinyint unsigned not null,       
	avg_temp float not null, 			
	num tinyint unsigned not null,		 
	primary key (time,drum)
)engine=InnoDB default charset=utf8;



create table TempThirdInfo
(
	time bigint unsigned not null,		  
	drum tinyint unsigned not null,       
	avg_temp float not null, 	  		
	num smallint unsigned not null,		
	primary key (time,drum)
)engine=InnoDB default charset=utf8;


create table UserLockInfo
(
	id smallint unsigned unique auto_increment,   
	lockid tinyint unsigned not null,			
	user_label varchar(10) not null, 			
	username varchar(20) not null ,      			
	foreign key (username) references UserDetail(username),   
	primary key(lockid,user_label)                  
)engine=InnoDB default charset=utf8;
alter table UserLockInfo auto_increment=1;


create table OpenLockInfo
(
	userlock_id smallint unsigned not null,
	lockid tinyint unsigned not null,	
	open_time bigint unsigned not null,		
	close_time bigint unsigned,				
	foreign key (userlock_id) references UserLockInfo(id),   
	primary key (userlock_id,open_time)
)engine=InnoDB default charset=utf8;



create table VideoInfo
(
	start_time bigint unsigned not null,    
	camera tinyint unsigned not null,			
	link varchar(100) not null,	
	primary key(start_time,camera)
)engine=InnoDB default charset=utf8;


use OilPrivateDB;

insert into UserDetail(username) values("oil001admin");
insert into UserDetail(username) values("oil001guest");
insert into UserDetail(username) values("oil001user");

insert into LevelFirstInfo(time,drum,level) values(201403180100,1,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180101,1,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180102,1,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180103,1,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180104,1,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180105,1,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180106,1,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180107,1,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180108,1,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180109,1,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180110,1,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180111,1,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180112,1,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180113,1,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180114,1,3201);

insert into LevelFirstInfo(time,drum,level) values(201403180100,2,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180101,2,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180102,2,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180103,2,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180104,2,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180105,2,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180106,2,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180107,2,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180108,2,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180109,2,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180110,2,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180111,2,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180112,2,3201);
insert into LevelFirstInfo(time,drum,level) values(201403180113,2,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180114,2,3203);

insert into LevelFirstInfo(time,drum,level) values(201403180100,3,3205);
insert into LevelFirstInfo(time,drum,level) values(201403180101,3,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180102,3,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180103,3,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180104,3,3205);
insert into LevelFirstInfo(time,drum,level) values(201403180105,3,3205);
insert into LevelFirstInfo(time,drum,level) values(201403180106,3,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180107,3,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180108,3,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180109,3,3205);
insert into LevelFirstInfo(time,drum,level) values(201403180110,3,3205);
insert into LevelFirstInfo(time,drum,level) values(201403180111,3,3203);
insert into LevelFirstInfo(time,drum,level) values(201403180112,3,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180113,3,3202);
insert into LevelFirstInfo(time,drum,level) values(201403180114,3,3205);

insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180100,1,3201,58);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180103,1,3205,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180105,1,3203,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180107,1,3201,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180109,1,3201,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180111,1,3204,54);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180113,1,3203,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180115,1,3201,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180117,1,3202,62);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180119,1,3205,60);

insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180101,2,3202,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180104,2,3205,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180105,2,3201,61);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180107,2,3206,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180108,2,3201,53);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180113,2,3205,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180114,2,3203,59);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180115,2,3203,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180117,2,3201,57);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180118,2,3205,60);

insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180102,3,3206,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180103,3,3202,55);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180104,3,3204,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180105,3,3202,63);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180106,3,3205,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180107,3,3203,61);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180109,3,3205,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180111,3,3201,59);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180112,3,3204,60);
insert into LevelSecondInfo(time,drum,avg_level,num) values(201403180114,3,3207,62);

insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180102,1,3206,610);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180103,1,3201,600);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180104,1,3202,619);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180105,1,3203,602);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180106,1,3202,608);

insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180101,2,3201,601);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180102,2,3201,610);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180103,2,3208,613);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180104,2,3203,612);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180107,2,3200,609);

insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180103,3,3203,611);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180104,3,3202,607);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180105,3,3205,611);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180107,3,3202,622);
insert into LevelThirdInfo(time,drum,avg_level,num) values(201403180109,3,3201,598);

insert into TempFirstInfo(time,drum,temp) values(201403180100,1,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180101,1,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180102,1,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180103,1,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180104,1,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180105,1,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180106,1,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180107,1,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180108,1,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180109,1,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180110,1,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180111,1,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180112,1,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180113,1,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180114,1,3201);

insert into TempFirstInfo(time,drum,temp) values(201403180100,2,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180101,2,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180102,2,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180103,2,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180104,2,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180105,2,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180106,2,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180107,2,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180108,2,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180109,2,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180110,2,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180111,2,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180112,2,3201);
insert into TempFirstInfo(time,drum,temp) values(201403180113,2,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180114,2,3203);

insert into TempFirstInfo(time,drum,temp) values(201403180100,3,3205);
insert into TempFirstInfo(time,drum,temp) values(201403180101,3,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180102,3,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180103,3,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180104,3,3205);
insert into TempFirstInfo(time,drum,temp) values(201403180105,3,3205);
insert into TempFirstInfo(time,drum,temp) values(201403180106,3,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180107,3,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180108,3,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180109,3,3205);
insert into TempFirstInfo(time,drum,temp) values(201403180110,3,3205);
insert into TempFirstInfo(time,drum,temp) values(201403180111,3,3203);
insert into TempFirstInfo(time,drum,temp) values(201403180112,3,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180113,3,3202);
insert into TempFirstInfo(time,drum,temp) values(201403180114,3,3205);

insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180100,1,32.01,58);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180103,1,32.05,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180105,1,32.03,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180107,1,32.01,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180109,1,32.01,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180111,1,32.04,54);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180113,1,32.03,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180115,1,32.01,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180117,1,32.02,62);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180119,1,32.05,60);

insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180101,2,32.02,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180104,2,32.05,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180105,2,32.01,61);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180107,2,32.06,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180108,2,32.01,53);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180113,2,32.05,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180114,2,32.03,59);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180115,2,32.03,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180117,2,32.01,57);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180118,2,32.05,60);

insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180102,3,32.06,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180103,3,32.02,55);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180104,3,32.04,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180105,3,32.02,63);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180106,3,32.05,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180107,3,32.03,61);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180109,3,32.05,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180111,3,32.01,59);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180112,3,32.04,60);
insert into TempSecondInfo(time,drum,avg_temp,num) values(201403180114,3,32.07,62);

insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180102,1,32.06,610);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180103,1,32.01,600);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180104,1,32.02,619);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180105,1,32.03,602);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180106,1,32.02,608);

insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180101,2,32.01,601);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180102,2,32.01,610);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180103,2,32.08,613);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180104,2,32.03,612);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180107,2,32.00,609);

insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180103,3,32.03,611);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180104,3,32.02,607);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180105,3,32.05,611);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180107,3,32.02,622);
insert into TempThirdInfo(time,drum,avg_temp,num) values(201403180109,3,32.01,598);

insert into UserLockInfo(lockid,user_label,username) values(1,"J0008","oil001admin");
insert into UserLockInfo(lockid,user_label,username) values(1,"J0006","oil001guest");
insert into UserLockInfo(lockid,user_label,username) values(1,"J0003","oil001user");
insert into UserLockInfo(lockid,user_label,username) values(2,"J0002","oil001admin");
insert into UserLockInfo(lockid,user_label,username) values(2,"J0008","oil001guest");
insert into UserLockInfo(lockid,user_label,username) values(2,"J0006","oil001user");

insert into OpenLockInfo(userlock_id,lockid,open_time) values(1,1,201403180101);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(2,1,201403180120);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(3,1,201403180170);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(3,1,201403180160);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(1,1,201403180299);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(2,1,201403180377);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(3,1,201403181101);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(1,1,201403182120);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(2,1,201403180770);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(2,1,201403180960);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(3,1,201403181299);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(1,1,201403181377);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(1,1,201403180191);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(3,1,201403185120);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(3,1,201403186170);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(2,1,201403184160);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(1,1,201403189299);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(2,1,201403190377);

insert into OpenLockInfo(userlock_id,lockid,open_time) values(4,2,201403183201);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403185320);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(6,2,201403187770);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(4,2,201403184260);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(6,2,201403186499);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403182477);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403185601);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403185620);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403186770);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(6,2,201403182460);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(4,2,201403185499);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403184577);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(6,2,201403186791);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(4,2,201403187620);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(6,2,201403183270);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(6,2,201403185660);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(5,2,201403185699);
insert into OpenLockInfo(userlock_id,lockid,open_time) values(4,2,201403197877);
