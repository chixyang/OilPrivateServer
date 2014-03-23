
/**数据库操作函数实现**/

#include "private_dbio.h"


//设置字符编码为utf8
#define mysql_setUTF8(x)    do{                                          	\
                              if(mysql_query(x,"set names \'utf8\'"))     	\
	                                 {                                      \
		                                    perror("set utf8 error");       \
		                                    recycleConn(x);                 \
											exit(1);						\
									 }	\
	                          }while(0)

// 获取当前系统时间
uint64 getCurrentTimeSeconds()
{
	return (uint64)(time((time_t *)NULL)); //获取当前国际标准时间，存的是总的秒数，因为time_t其实是int，所以所存储最大时间到2038年
}

//添加用户详细信息
int addUserDetail(char *username,char *password,uint32 phone,char *rname)
{
	if(username == NULL)
		return -1;
	MYSQL *conn = getIdleConn();
	unsigned long affected_rows = 0;   //改变的语句数目
	char *sql_str = NULL;   //sql语句
  
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"insert into UserDetail(username,password,cellphone,real_name) values('%s','%s',%4u,'%s')", \
	         username,password,phone,rname);
	//执行插入并判断插入是否成功
	if(mysql_query(conn,sql_str) || ((affected_rows = mysql_affected_rows(conn)) < 1))
	{
		debug("add user detail error\n");
		recycleConn(conn);
		free(sql_str);
		return -1;
	}
   
	//插入成功     
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//获取用户单个信息,获取的信息空间需要显式释放,如果获取的是int，则需要自己调用atoi函数转换
char *getUserDetail(char *username,enum UserDetail type)
{
	if(username == NULL)
		return NULL;
	
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	switch(type)
	{
		case USER_PASSWORD: sprintf(sql_str,"select password from UserDetail where username = '%s'",username); break;
		case USER_PHONE: sprintf(sql_str,"select cellphone from UserDetail where username = '%s'",username); break;
		case USER_RNAME: sprintf(sql_str,"select real_name from UserDetail where username = '%s'",username); break;
		default : 
			 debug("get user detail unrecognized  cmd error\n");
			 recycleConn(conn);
			 free(sql_str);
			 return NULL;
	}
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("get user detail error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	//如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		int len = strlen(row[0]);
		char *info = (char *)malloc(len + 1); //加上结束位
		memcpy(info,row[0],len);
		info[len] = '\0'; //加上结束位
		//释放资源
		mysql_free_result(res);
		recycleConn(conn);
		free(sql_str);
		return info;
	}
	//未查到数据
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return NULL;
}

//更新用户信息,int的话就强制转为指针,譬如a为整数，则传参为（void *）a
int updateUserDetail(char *username,enum UserDetail type,void *value)
{
	if((NULL == username) || (NULL == value))
		return -1;

	MYSQL *conn = getIdleConn();
	unsigned long affected_rows = 0;   //改变的语句数目
	char *sql_str = NULL;   //sql语句
  
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	switch(type)
	{
		case USER_PASSWORD: sprintf(sql_str,"update UserDetail set password = '%s' where username = '%s'",(char *)value,username); break;
		case USER_PHONE: sprintf(sql_str,"update UserDetail set cellphone = %4u where username = '%s'",(uint32)value,username);break;
		case USER_RNAME: sprintf(sql_str,"update UserDetail set real_name = '%s' where username = '%s'",(char *)value,username);break;
		default : 			 
			 debug("update user detail unrecognized  cmd error\n");
			 recycleConn(conn);
			 free(sql_str);
			 return -1;
	}
	//执行插入并判断插入是否成功
	int ret_query = mysql_query(conn,sql_str);       //不插入二进制数据，就不用担心mysql_query的问题
	affected_rows = mysql_affected_rows(conn); 
	if(ret_query  || (affected_rows < 1))
	{
		debug("update user detail error");
		recycleConn(conn);
		free(sql_str);
		return -1;
	}
      
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//添加Level和Temp数据,桶的编号不能从0算起,这种传递结构的方法虽然简单，但是耗时，在本项目中，时间不是重要参数，所以可以采用此方法
int addLTInfo(LTInfo *info,enum LTLabel type)    
{
	if((info == NULL) ||(info->time == (uint64)0) || (info->drum == (uint8)0) || ((info->lt.level == (uint16)0) && ((info->lt.temp == .0f))))
	{
		if(info == NULL)
			debug("info = NULL\n");
		else
			debug("time : %llu,drum : %1u,level : %2u,temp : %f\n",info->time,info->drum,info->lt.level,info->lt.temp);
		return -1;
	}
	MYSQL *conn = getIdleConn();
	unsigned long affected_rows = 0;   //改变的语句数目
	char *sql_str = NULL;   //sql语句
  
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	switch(type)
	{
		case L_FIRST: sprintf(sql_str,"insert into LevelFirstInfo(time,drum,level) values(%llu,%1u,%2u)",/*如果cu不行就用1u,hu也可以用2u表示*/info->time,info->drum,info->lt.level); break;
		case L_SECOND: sprintf(sql_str,"insert into LevelSecondInfo(time,drum,avg_level,num) values(%llu,%1u,%2u,%1u)",info->time,info->drum,info->lt.level,info->num); break;
		case L_THIRD: sprintf(sql_str,"insert into LevelThirdInfo(time,drum,avg_level,num) values(%llu,%1u,%2u,%2u)",info->time,info->drum,info->lt.level,info->num); break;
		case T_FIRST: sprintf(sql_str,"insert into TempFirstInfo(time,drum,temp) values(%llu,%1u,%.1f)",info->time,info->drum,info->lt.temp); break;
		case T_SECOND: sprintf(sql_str,"insert into TempSecondInfo(time,drum,avg_temp,num) values(%llu,%1u,%.1f,%1u)",info->time,info->drum,info->lt.temp,info->num); break;
		case T_THIRD: sprintf(sql_str,"insert into TempThirdInfo(time,drum,avg_temp,num) values(%llu,%1u,%.1f,%2u)",info->time,info->drum,info->lt.temp,info->num); break;
		default:
			 debug("unrecognized  cmd error\n");
			 recycleConn(conn);
			 free(sql_str);
			 return -1;
	}
	
	//执行插入并判断插入是否成功
	if(mysql_query(conn,sql_str) || ((affected_rows = mysql_affected_rows(conn)) < 1))
	{
		debug("db add level temp info error,error: %s   affected rows : %lu\n",mysql_error(conn),affected_rows);
		debug("type = %d\n",type);
		recycleConn(conn);
		free(sql_str);
		return -1;
	}
   
	//插入成功     
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//获取单个油桶的液位或温度信息（多个油桶的话则由接收协议判断并多次调用该函数），不要把所有的方法写到一起，否则严重影响效率，最多像下面这样写
LTInfo *getLTInfo(uint64 starttime,uint8 drum,uint8 num,enum LTLabel type)
{
	if((starttime == (uint64)0) || (drum == (uint8)0) || (num == (uint8)0))
		return NULL;
	switch(type)
	{
		case L_FIRST: return getLevelFirstInfo(starttime,drum,num);    //获取第一级液位信息
		case L_SECOND: 
		case L_THIRD: return getLevelSTInfo(starttime,drum,num,type);  //获取第二三级液位信息
		case T_FIRST: return getTempFirstInfo(starttime,drum,num);		//获取第一级温度信息
		case T_SECOND: 
		case T_THIRD: return getTempSTInfo(starttime,drum,num,type);	//获取第二三级温度信息
		default :
			 debug("get lt type error\n");
			 return NULL;
	}
}

//获取第一级液位信息
LTInfo *getLevelFirstInfo(uint64 starttime,uint8 drum,uint8 num)
{
	if((starttime == (uint64)0) || (drum == (uint8)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	
	sprintf(sql_str,"select * from LevelFirstInfo where time > %llu and drum = %1u order by time limit %1u",starttime,drum,num);
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("get level first info error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	LTInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	 //如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (LTInfo *)malloc(sizeof(LTInfo));
		memset(curInfo,0,sizeof(LTInfo));
		curInfo->time = atoll(row[0]);
		curInfo->drum = (uint8)atoi(row[1]);
		curInfo->lt.level = (uint16)atoi(row[2]);
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (LTInfo *)malloc(sizeof(LTInfo));
			memset(curInfo,0,sizeof(LTInfo));
			curInfo->time = atoll(row[0]);
			curInfo->drum = (uint8)atoi(row[1]);
			curInfo->lt.level = (uint16)atoi(row[2]);
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//获取第二、三级液位信息
LTInfo *getLevelSTInfo(uint64 starttime,uint8 drum,uint8 num,enum LTLabel type)
{
	if((starttime == (uint64)0) || (drum == (uint8)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	switch(type)
	{
		case L_SECOND: sprintf(sql_str,"select * from LevelSecondInfo where time > %llu and drum = %1u order by time limit %1u",/*这句代码查出来的刚刚好是离time最近的几个数据，所以满足需求*/starttime,drum,num); break;
		case L_THIRD: sprintf(sql_str,"select * from LevelThirdInfo where time > %llu and drum = %1u order by time limit %1u",starttime,drum,num); break;
		default :
			 debug("get LevelST type error\n");
			 recycleConn(conn);
			 free(sql_str);
			 return NULL;
	}
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("get LevelST info error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	LTInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	 //如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (LTInfo *)malloc(sizeof(LTInfo));
		memset(curInfo,0,sizeof(LTInfo));
		curInfo->time = atoll(row[0]);
		curInfo->drum = (uint8)atoi(row[1]);
		curInfo->lt.level = (uint16)atoi(row[2]);
		curInfo->num = (uint16)atoi(row[3]);
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (LTInfo *)malloc(sizeof(LTInfo));
			memset(curInfo,0,sizeof(LTInfo));
			curInfo->time = atoll(row[0]);
			curInfo->drum = (uint8)atoi(row[1]);
			curInfo->lt.level = (uint16)atoi(row[2]);
			curInfo->num = (uint16)atoi(row[3]);
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//获取第一级温度信息
LTInfo *getTempFirstInfo(uint64 starttime,uint8 drum,uint8 num)
{
	if((starttime == (uint64)0) || (drum == (uint8)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	
	sprintf(sql_str,"select * from TempFirstInfo where time > %llu and drum = %1u order by time limit %1u",starttime,drum,num);
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("get temp first info error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	LTInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	 //如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (LTInfo *)malloc(sizeof(LTInfo));
		memset(curInfo,0,sizeof(LTInfo));
		curInfo->time = atoll(row[0]);
		curInfo->drum = (uint8)atoi(row[1]);
		curInfo->lt.temp = atof(row[2]);
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (LTInfo *)malloc(sizeof(LTInfo));
			memset(curInfo,0,sizeof(LTInfo));
			curInfo->time = atoll(row[0]);
			curInfo->drum = (uint8)atoi(row[1]);
			curInfo->lt.temp = atof(row[2]);
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//获取第二、三级温度信息
LTInfo *getTempSTInfo(uint64 starttime,uint8 drum,uint8 num,enum LTLabel type)
{
	if((starttime == (uint64)0) || (drum == (uint8)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	switch(type)
	{
		case T_SECOND: sprintf(sql_str,"select * from TempSecondInfo where time > %llu and drum = %1u order by time limit %1u",starttime,drum,num); break;
		case T_THIRD: sprintf(sql_str,"select * from TempThirdInfo where time > %llu and drum = %1u order by time limit %1u",starttime,drum,num); break;
		default :
			 debug("get TempST type error\n");
			 recycleConn(conn);
			 free(sql_str);
			 return NULL;
	}
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("get TempST info error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	LTInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	 //如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (LTInfo *)malloc(sizeof(LTInfo));
		memset(curInfo,0,sizeof(LTInfo));
		curInfo->time = atoll(row[0]);
		curInfo->drum = (uint8)atoi(row[1]);
		curInfo->lt.temp = atof(row[2]);
		curInfo->num = (uint16)atoi(row[3]);
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (LTInfo *)malloc(sizeof(LTInfo));
			memset(curInfo,0,sizeof(LTInfo));
			curInfo->time = atoll(row[0]);
			curInfo->drum = (uint8)atoi(row[1]);
			curInfo->lt.temp = atof(row[2]);
			curInfo->num = (uint16)atoi(row[3]);
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//添加用户锁信息
int addUserLockInfo(uint8 lock,char *label,char *username)
{
	if((lock == (uint8)0) || (label == NULL) || (username == NULL))
		return -1;
	MYSQL *conn = getIdleConn();
	unsigned long affected_rows = 0;   //改变的语句数目
	char *sql_str = NULL;   //sql语句
  
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"insert into UserLockInfo(lockid,user_label,username) values(%1u,'%s','%s')", \
	         lock,label,username);
	//执行插入并判断插入是否成功
	if(mysql_query(conn,sql_str) || ((affected_rows = mysql_affected_rows(conn)) < 1))
	{
		debug("add user lock info error\n");
		recycleConn(conn);
		free(sql_str);
		return -1;
	}
    
	//插入成功     
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//通过锁编号和用户名(锁用户编号)获取锁记录id,user既可能为username也可能为userlabel
uint16 getIDbyUserLock(uint8 lock,char *user,enum UserLock type)
{
	if((lock == (uint8)0) || (user == NULL))
		return (uint16)0;
	
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	switch(type)
	{
		case LOCK_USERLABEL: sprintf(sql_str,"select id from UserLockInfo where lockid = %1u and user_label = '%s'", \
						lock,user);break;
		case LOCK_USERNAME: sprintf(sql_str,"select id from UserLockInfo where lockid = %1u and username = '%s'", \
						lock,user);break;
		default:
				debug("getIDbyUserLock type error\n");
				recycleConn(conn);
				free(sql_str);
				return (uint16)0;
	}
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("getIDbyUserLock error\n");
		recycleConn(conn);
		free(sql_str);
		return (uint16)0;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	//如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		uint16 id = (uint16)atoi(row[0]);
		//释放资源
		mysql_free_result(res);
		recycleConn(conn);
		free(sql_str);
		return id;
	}
	//未查到数据
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//添加开锁信息OpenLockInfo
int addOpenLockInfo(uint16 userlock_id,uint8 lock,uint64 open_time)
{
	if((userlock_id == (uint16)0) || (lock == (uint8)0) || (open_time == (uint64)0))
		return -1;
	MYSQL *conn = getIdleConn();
	unsigned long affected_rows = 0;   //改变的语句数目
	char *sql_str = NULL;   //sql语句
  
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"insert into OpenLockInfo(userlock_id,lockid,open_time) values(%2u,%1u,%llu)",userlock_id,lock,open_time);
	//执行插入并判断插入是否成功
	if(mysql_query(conn,sql_str) || ((affected_rows = mysql_affected_rows(conn)) < 1))
	{
		debug("add lock info error\n");
		recycleConn(conn);
		free(sql_str);
		return -1;
	}
   
	//插入成功     
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//更新开锁信息
int updateOpenLockInfo(uint16 userlock_id,uint64 open_time,uint64 close_time)
{
	if((userlock_id == (uint16)0) || ((uint64)0 == open_time))
		return -1;

	MYSQL *conn = getIdleConn();
	unsigned long affected_rows = 0;   //改变的语句数目
	char *sql_str = NULL;   //sql语句
  
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"update OpenLockInfo set close_time = %llu where userlock_id = %2u and open_time = %llu",close_time,userlock_id,open_time);
	//执行插入并判断插入是否成功
	int ret_query = mysql_query(conn,sql_str);       //不插入二进制数据，就不用担心mysql_query的问题
	affected_rows = mysql_affected_rows(conn); 
	if(ret_query  || (affected_rows < 1))
	{
		debug("update open lock info error");
		recycleConn(conn);
		free(sql_str);
		return -1;
	}
      
	recycleConn(conn);
	free(sql_str);
	return 0;
}

//通过锁记录id查锁信息,通过这个查询的本来就不需要name，name都一样，直接用传输协议的参数
OLInfo *getOLInfoByID(uint16 userlock_id,uint64 starttime,uint8 num)
{
//	debug("in get ol info byid\n");
	if((userlock_id == (uint16)0) || (starttime == (uint64)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"select open_time,close_time from OpenLockInfo where open_time > %llu and userlock_id = %2u order by open_time limit %1u",starttime,userlock_id,num);
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("getOLInfoByID error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	OLInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	//如果查询结果不为空
//	debug("middle get ol info by id\n");
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (OLInfo *)malloc(sizeof(OLInfo));
		memset(curInfo,0,sizeof(OLInfo));
		curInfo->open_time = (uint64)atoll(row[0]);
		if(row[1] == NULL)
			curInfo->close_time = 0;
		else
			curInfo->close_time = (uint64)atoll(row[1]);
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (OLInfo *)malloc(sizeof(OLInfo));
			memset(curInfo,0,sizeof(OLInfo));
			curInfo->open_time = (uint64)atoll(row[0]);
			if(row[1] == NULL)
				curInfo->close_time = 0;
			else
				curInfo->close_time = (uint64)atoll(row[1]);
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
//	debug("after getolinfobyid \n");
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//通过锁号查锁信息,数据库表合并查询。。。,就避免查两次了
OLInfo *getOLInfoByLock(uint8 lock,uint64 starttime,uint8 num)   
{
	if((lock == (uint8)0) || (starttime == (uint64)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"select username,open_time,close_time from OpenLockInfo,UserLockInfo where open_time > %llu and OpenLockInfo.lockid = %1u and userlock_id = id order by open_time limit %1u",starttime,lock,num);
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("getOLInfoByLock error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	OLInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	 //如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (OLInfo *)malloc(sizeof(OLInfo));
		memset(curInfo,0,sizeof(OLInfo));
		curInfo->open_time = (uint64)atoll(row[1]);
		if(row[2] != NULL)
			curInfo->close_time = (uint64)atoll(row[2]);
		else
			curInfo->close_time = 0;
		memcpy(curInfo->username,row[0],strlen(row[0]));
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (OLInfo *)malloc(sizeof(OLInfo));
			memset(curInfo,0,sizeof(OLInfo));
			curInfo->open_time = (uint64)atoll(row[1]);
			if(row[2] != NULL)
				curInfo->close_time = (uint64)atoll(row[2]);
			else
				curInfo->close_time = 0;
			memcpy(curInfo->username,row[0],strlen(row[0]));
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//仅按照时间顺序获取一定数量的开锁记录,当lock为全0，openuser为null时候
OLInfo *getOLInfoByTime(uint64 starttime,uint8 num)
{
	if((starttime == (uint64)0) || (num == (uint8)0))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"select OpenLockInfo.lockid,username,open_time,close_time from OpenLockInfo,UserLockInfo where open_time > %llu and userlock_id = id order by open_time limit %1u",starttime,num);
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("getOLInfoByID error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	OLInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	//如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (OLInfo *)malloc(sizeof(OLInfo));
		memset(curInfo,0,sizeof(OLInfo));
		curInfo->lock = (uint8)atoi(row[0]);
		curInfo->open_time = (uint64)atoll(row[2]);
		if(row[3] != NULL)
			curInfo->close_time = (uint64)atoll(row[3]);
		else
			curInfo->close_time = 0;
		memcpy(curInfo->username,row[1],strlen(row[1]));
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (OLInfo *)malloc(sizeof(OLInfo));
			memset(curInfo,0,sizeof(OLInfo));
			curInfo->lock = (uint8)atoi(row[0]);
			curInfo->open_time = (uint64)atoll(row[2]);
			if(row[3] != NULL)
				curInfo->close_time = (uint64)atoll(row[3]);
			else
				curInfo->close_time = 0;
			memcpy(curInfo->username,row[1],strlen(row[1]));
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}

//仅按照时间顺序和用户名获取一定数量的开锁记录,当lock为全0，openuser不为null时候
OLInfo *getOLInfoByTimeUser(uint64 starttime,uint8 num,char *username)
{
	if((starttime == (uint64)0) || (num == (uint8)0) || (username == NULL))
		return NULL;
	MYSQL *conn = getIdleConn();
	MYSQL_RES *res;      //查询的result
	MYSQL_ROW row;
	char *sql_str = NULL;   //sql语句
	
	//设置字符编码为utf8
	mysql_setUTF8(conn);
	//设置插入语句
	sql_str = (char *)malloc(sizeof(char) * 200);
	memset(sql_str,0,200);
	sprintf(sql_str,"select OpenLockInfo.lockid,open_time,close_time from OpenLockInfo,UserLockInfo where open_time > %llu and userlock_id = id and username='%s' order by open_time limit %1u",starttime,username,num);
	//执行查询
	if(mysql_query(conn,sql_str))
	{
		debug("getOLInfoByID error\n");
		recycleConn(conn);
		free(sql_str);
		return NULL;
	}
	//获取查询结果
	res = mysql_use_result(conn);
	OLInfo *info = NULL, *preInfo = NULL,*curInfo = NULL;
	//如果查询结果不为空
	if((row = mysql_fetch_row(res)) != NULL)
	{
		curInfo = (OLInfo *)malloc(sizeof(OLInfo));
		memset(curInfo,0,sizeof(OLInfo));
		curInfo->lock = (uint8)atoi(row[0]);
		curInfo->open_time = (uint64)atoll(row[1]);
		if(row[2] != NULL)
			curInfo->close_time = (uint64)atoll(row[2]);
		else
			curInfo->close_time = 0;
		info = curInfo;
		preInfo = curInfo;
		while((row = mysql_fetch_row(res)) != NULL)
		{
			curInfo = (OLInfo *)malloc(sizeof(OLInfo));
			memset(curInfo,0,sizeof(OLInfo));
			curInfo->lock = (uint8)atoi(row[0]);
			curInfo->open_time = (uint64)atoll(row[1]);
			if(row[2] != NULL)
				curInfo->close_time = (uint64)atoll(row[2]);
			else 
				curInfo->close_time = 0;
			preInfo->next = curInfo;
			preInfo = preInfo->next;
		}
	}
	//释放资源
	mysql_free_result(res);
	recycleConn(conn);
	free(sql_str);
	return info;
}


//释放LTInfo列表
void freeLTInfoList(LTInfo *info)
{
	if(info != NULL)
	{
		LTInfo * tmp = info->next;
		while(tmp != NULL)
		{
			info->next = tmp->next;
			free(tmp);
			tmp = info->next;
		}
		free(info);
	}
}

//释放OLInfo
void freeOLInfoList(OLInfo *info)
{
	if(info != NULL)
	{
		OLInfo * tmp = info->next;
		while(tmp != NULL)
		{
			info->next = tmp->next;
			free(tmp);
			tmp = info->next;
		}
		free(info);
	}
}

