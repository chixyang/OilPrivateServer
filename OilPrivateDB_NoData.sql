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

