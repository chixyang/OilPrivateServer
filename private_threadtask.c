
/** 线程任务处理函数实现 **/

#include "private_threadtask.h"

#define ReportPeriod 60    //测试时候改为10,正确应该设置为60,向代理服务器汇报自己ip和port的时间段,单位秒

//代理服务器ip地址
char *agent_ip = "117.121.26.161";
//代理服务器端口
uint16 agent_port = 3301;
//自己账户
char * self_account = "oil001";

//协议解析函数
void *parseCommand(void *arg)
{
	RecvCommand *cmd = (RecvCommand *)arg;
	/* 校验数据长度是否对的上，校验码是否正确 */
	char length[size_Length];
	memcpy(length,cmd->buf + size_SerialNum,size_Length);
	printf("length : %d\n",*(uint16 *)length);
	if((cmd->buflen - size_SerialNum) != *((uint16 *)length))  //长度对不上
	{
			free(arg);
			return NULL;
	}
	
	//检验校验码
	uint8 CheckSum = checkSum(cmd->buf,cmd->buflen);
	debug("check sum : %1u\n",CheckSum);
	if(CheckSum) //校验错误
	{
			free(arg);
			return NULL;
	}
	
	//正确协议
	uint8 type = *(uint8 *)(cmd->buf + size_SerialNum + size_Length);
	//获取序列码
	char serial[size_SerialNum];
	memcpy(serial,cmd->buf,size_SerialNum);
	//获取data部分
	char *data = (char *)(cmd->buf + size_SerialNum + size_Length + size_Type);
	if(type == CMD_REQLEVEL)       //请求历史液位信息
	{	
		debug("in cmd req history level\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		char *str_starttime = str_drum + strlen(str_drum) + size_Split;
		uint64 starttime = (uint64)atoll(str_starttime);
		char *str_NumofItems = str_starttime + strlen(str_starttime) + size_Split;
		uint8 NumofItems = (uint8)atoi(str_NumofItems);
		char* str_period = (str_NumofItems + strlen(str_NumofItems) + size_Split);
		uint8 period = (uint8)atoi(str_period);
	//	debug("name : %s,drum : %s,starttime : %s,numofitems : %s,period : %1u\n",username,str_drum,str_starttime,str_NumofItems,(uint8)period);
		char ltype = LT_NONE;
		//选择级别参数
		if(period == 1)
			ltype = L_FIRST;
		else if(period == 2)
			ltype = L_SECOND;
		else if(period == 3)
			ltype = L_THIRD;
		else
		{
			debug("unknown cmd,period = %d\n",period);
			uint16 len = 0;
			char *buf = getRpyErrorReq(serial,type,username,&len);
			InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
			free(buf);
			free(arg);
			return NULL;
		}
		if(drum != 0)  //单一油桶查询
		{
			LTInfo *info = getLTInfo(starttime,drum,NumofItems,ltype);
			if(info == NULL)
			{
				debug("get level history error\n");
				uint16 len = 0;
				char *buf = getRpyErrorReq(serial,type,username,&len);
				InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
				free(buf);
				free(arg);
				return NULL;
			}
			uint16 len = 0;
			char *buf = getRpyLevelHistory(username,serial,info,&len);
			if(buf == NULL)
			{
				debug("get level history error\n");
				freeLTInfoList(info);
				uint16 length = 0;
				char *buff = getRpyErrorReq(serial,type,username,&length);
				InSendQueue(cmd->sockfd,&(cmd->recv_addr),buff,length);
				free(buff);
				free(arg);
				return NULL;
			}
			freeLTInfoList(info);
			InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
			free(buf);
		}
		else{		//多桶油查询，多桶油查询的话，协议里的NumofItems表示什么
				//暂时未做
		}
	//	debug("cmd after req level\n");
	}
	else if(type == CMD_REQTEMP)   //请求历史温度信息
	{
		debug("cmd in req history temp\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		char *str_starttime = str_drum + strlen(str_drum) + size_Split;
		uint64 starttime = (uint64)atoll(str_starttime);
		char *str_NumofItems = str_starttime + strlen(str_starttime) + size_Split;
		uint8 NumofItems = (uint8)atoi(str_NumofItems);
		char *str_period = str_NumofItems + strlen(str_NumofItems) + size_Split;
		uint8 period = (uint8)atoi(str_period);
		char ltype = LT_NONE;
		//选择级别参数
		if(period == 1)
			ltype = T_FIRST;
		else if(period == 2)
			ltype = T_SECOND;
		else if(period == 3)
			ltype = T_THIRD;
		else
		{
			debug("unknown cmd");
			uint16 len = 0;
			char *buf = getRpyErrorReq(serial,type,username,&len);
			InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
			free(buf);
			free(arg);
			return NULL;
		}
		if(drum != 0)  //单一油桶查询
		{
			LTInfo *info = getLTInfo(starttime,drum,NumofItems,ltype);
			if(info == NULL)
			{
				debug("get temp history error\n");
				uint16 len = 0;
				char *buf = getRpyErrorReq(serial,type,username,&len);
				InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
				free(buf);
				free(arg);
				return NULL;
			}
			uint16 len = 0;
			char *buf = getRpyTempHistory(username,serial,info,&len);
			if(buf == NULL)
			{
				debug("get temp history error\n");
				freeLTInfoList(info);
				uint16 length = 0;
				char *buff = getRpyErrorReq(serial,type,username,&length);
				InSendQueue(cmd->sockfd,&(cmd->recv_addr),buff,length);
				free(buff);
	free(arg);
				return NULL;
			}
			freeLTInfoList(info);
			InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
			free(buf);
		}
		else{		//多桶油查询，多桶油查询的话，协议里的NumofItems表示什么
				//暂时未做
		}
	//	debug("cmd after req temp\n");
	}
	else if(type == CMD_REQLOCK)    //请求历史开锁信息
	{
	//	debug("cmd in req lock\n");
		//获取协议参数
		char *username = data;
		char *str_lock = username + strlen(username) + size_Split;
		uint8 lock = (uint8)atoi(str_lock);
		char *str_starttime = str_lock + strlen(str_lock) + size_Split;
		uint64 starttime = (uint64)atoll(str_starttime);
		char *str_NumofItems = str_starttime + strlen(str_starttime) + size_Split;
		uint8 NumofItems = (uint8)atoi(str_NumofItems);
		//将协议的校验位设置为0，即'\0'
		cmd->buf[cmd->buflen - 1] = '\0';
		char *openuser = str_NumofItems + strlen(str_NumofItems) + size_Split; //如果协议里面没有openuser值，则openuser值为校验位值，即为'\0'
		debug("name : %s, lock : %s, starttime : %s,nums : %s ,openuser : %s,strlen(openuser) : %d\n",username,str_lock,str_starttime,str_NumofItems,openuser,strlen(openuser));
		OLInfo *info = NULL;
		if(*openuser == '\0')   //不根据用户名查询
		{
			if(lock == 0) //查询时间条件下的所有锁信息
				info = getOLInfoByTime(starttime,NumofItems);
			else  //查询某个时间下特定锁的信息
			{
				debug("before get ol info\n");
				info = getOLInfoByLock(lock,starttime,NumofItems);
				OLInfo *tmp = info;  //因为info的lock为空，所以赋值
				while(tmp)
				{
					tmp->lock = lock;
					tmp = tmp->next;
				}
				//debug("after\n");
			}
		}
		else
		{
			if(lock == 0)   //查询某个用户对所有锁的操作记录
			{
				info = getOLInfoByTimeUser(starttime,NumofItems,openuser);
				OLInfo *tmp = info;
				int len = strlen(openuser);
				while(tmp)
				{
					memcpy(tmp->username,openuser,len);
					tmp = tmp->next;
				}
			}
			else		//查询某个用户对特定锁的操作记录
			{
				uint16 userlock_id = getIDbyUserLock(lock,openuser,LOCK_USERNAME);
	//			debug("userlock_id : %d, starttime : %llu, nums : %1u\n",userlock_id,starttime,NumofItems);
				info = getOLInfoByID(userlock_id,starttime,NumofItems);
				//debug("info : %d\n",(int)info);
				OLInfo *tmp = info;
				int len = strlen(openuser);
				while(tmp)
				{
					tmp->lock = lock;
					memcpy(tmp->username,openuser,len);
					tmp = tmp->next;
				}
			}
		}
		if(info == NULL)
		{
			debug("get lock info error\n");
			uint16 len = 0;
			char *buf = getRpyErrorReq(serial,type,username,&len);
			InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
			free(buf);
			free(arg);
			return NULL;
		}
		uint16 len = 0;
	//	debug("before get reply\n");
		char *buf = getRpyLockHistory(username,serial,info,&len);
		if(buf == NULL)
		{
			debug("get temp history error\n");
			freeOLInfoList(info);
			uint16 length = 0;
			char *buff = getRpyErrorReq(serial,type,username,&length);
			InSendQueue(cmd->sockfd,&(cmd->recv_addr),buff,length);
			free(buff);
			free(arg);
			return NULL;
		}
		freeOLInfoList(info);
		InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
		free(buf);
	//	debug("after cmd in req lock\n");
	}
	else if(type == CMD_REQVIDEO)  //视频请求暂未做
	{
		free(arg);
		return NULL;
	}
	else if(type == CMD_REQHOLE || type == CMD_RPYHOLE)  //收到打洞或停止打洞请求协议，表明打洞成功，取消打洞超时发送协议，并且发送打洞停止协议一次
	{
	//	debug("cmd in req hole\n");
		//取消打洞协议的超时重发机制
		QuitReSendQueue(&(cmd->recv_addr),RESEND_HOLE,0);
		//发送停止打洞协议
		uint16 len = 0;
		char *buf = getStopHole(serial,&len);
		if(buf == NULL) //这个就不做错误回复，不回复效果更好
		{
			debug("get stop hole error\n");
			free(arg);
			return NULL;
		}
		InSendQueue(cmd->sockfd,&(cmd->recv_addr),buf,len);
		free(buf);
	//	debug("cmd after req hole");
	}
	else if(type == CMD_ALARM)  //收到报警协议，表明客户端成功收到报警信息，服务器仅取消报警超时发送机制
	{
	//	debug("cmd in req alarm\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		QuitReSendQueue(&(cmd->recv_addr),RESEND_ALARM,drum);
	//	debug("cmd after req alarm\n");
	}
	else if(type == CMD_LEVELINOUT)  //收到该协议则表明客户端成功收到该通知,服务器仅取消液位节点进入退出超时发送机制
	{
	//	debug("cmd in level in out\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		char *str_time = str_drum + strlen(str_drum) + size_Split;
		uint64 curtime = (uint64)atoll(str_time);
		char *str_type = str_time + strlen(str_time) + size_Split;
		uint8 type = (uint8)atoi(str_type);
		if(type == 0)  //0表示节点退出
			QuitReSendQueue(&(cmd->recv_addr),RESEND_LEVEL_NODEOFF,drum);
		else if(type == 1)
			QuitReSendQueue(&(cmd->recv_addr),RESEND_LEVEL_NODEON,drum);
	//	debug("cmd after level in out\n");
	}
	else if(type == CMD_TEMPINOUT)  //收到该协议则表明客户端成功收到该通知,服务器仅取消温度节点进入退出超时发送机制
	{
	//	debug("cmd in temp in out\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		char *str_time = str_drum + strlen(str_drum) + size_Split;
		uint64 curtime = (uint64)atoll(str_time);
		char *str_type = str_time + strlen(str_time) + size_Split;
		uint8 type = (uint8)atoi(str_type);
		if(type == 0)  //0表示节点退出
			QuitReSendQueue(&(cmd->recv_addr),RESEND_TEMP_NODEOFF,drum);
		else if(type == 1)
			QuitReSendQueue(&(cmd->recv_addr),RESEND_TEMP_NODEON,drum);
	//	debug("cmd after temp in out\n");
	}
	else if(type == CMD_REQREALLT)  //液位和温度实时请求协议，服务器收到后，将该节点加入实时请求列表
	{
	//	debug("cmd in req real list\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		int ret = InRealSendList(username,drum,cmd->sockfd,&(cmd->recv_addr), REAL_LTINFO_TRANS);
	//	debug("in real send list ret : %d\n",ret);
	//	debug("cmd after req real list\n");
	}
	else if(type == CMD_CNLREALLT)
	{
	//	debug("cmd in cnl real lt\n");
		//获取协议参数
		char *username = data;
		char *str_drum = username + strlen(username) + size_Split;
		uint8 drum = (uint8)atoi(str_drum);
		QuitRealSendList(username,drum,REAL_LTINFO_TRANS);
	//	debug("cmd after cnl real lt\n");
	}
	else if(type == CMD_RPYLOGIN)
	{
	//	debug("cmd in rpl login\n");
		//获取协议参数
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		char *str_ip = data;
		addr.sin_addr.s_addr = inet_addr(str_ip);
		char *str_port = str_ip + strlen(str_ip) + size_Split;
		addr.sin_port = htons((uint16)atoi(str_port));
		//获取打洞协议
		uint16 len = 0;
		char *buf = getHole(&len);
		//加入超时重发列表，开始打洞
		InTimelyReSendQueue(cmd->sockfd,&addr,buf,len,RESEND_HOLE,0);
	//	debug("cmd after rpl login\n");
	}
	else if(type == CMD_LTREPORT)   //液位温度上传
	{
		//debug("cmd in lt report\n");
		char *drum_str = data;
		uint8 drum = (uint8)atoi(drum_str);
		debug("drum is %d\n",drum);
		char *level_str = drum_str + strlen(drum_str) + size_Split;
		char *temp_str = level_str + strlen(level_str) +size_Split;
		debug("recv lt drum : %s  level : %s  temp : %s\n",drum_str,level_str,temp_str);
		uint16 level = (uint16)atoi(level_str);
		debug("level is %d\n",level);
		float temp = atoi(temp_str)/10.0;
		debug("temp is %f\n",temp);
		//加入新数据列表
		addNewLTInfo(L_FIRST,&level,drum);
		addNewLTInfo(T_FIRST,&temp,drum);
		//debug("cmd after lt report\n");
	}
	else if(type == CMD_OLREPORT)  //开锁信息上传
	{
	//	debug("cmd in ol report\n");
		char *lock_str = data;
		uint8 lock = (uint8)atoi(lock_str);
		char *user_label = lock_str + strlen(lock_str) + size_Split;
		uint16 userlock_id = getIDbyUserLock(lock,user_label,LOCK_USERLABEL);
	//	debug("recv ol lock : %s  userlabel : %s  userlock_id : %2u\n",lock_str,user_label,userlock_id);
		if(userlock_id == 0)
		{
			debug("can not find userlabel in lock\n");
			free(arg);
			return NULL;
		}
		uint64 open_time = getCurrentTimeSeconds();
		addOpenLockInfo(userlock_id,lock,open_time);
	//	debug("cmd after ol report\n");
	}
	
	free(arg);
	return NULL;
}

ReportInfo* initReportInfo(int sockfd)
{
	if(sockfd <= 0)
	{
		debug("sockfd %d\n",sockfd);
		return NULL;
	}
	ReportInfo *RInfo = (ReportInfo *)malloc(sizeof(ReportInfo));
	memset(RInfo,0,sizeof(ReportInfo));
	RInfo->sockfd = sockfd;
	//配置agentaddr
	RInfo->agentaddr.sin_family = AF_INET;
	RInfo->agentaddr.sin_addr.s_addr = inet_addr(agent_ip);
	RInfo->agentaddr.sin_port = htons(agent_port);
	//获取msg及len
	RInfo->len = size_SerialNum + size_Length + size_Type + strlen(self_account) + size_Split + size_CheckSum;
	RInfo->msg = (char *)malloc((RInfo->len) * sizeof(char));
	memset(RInfo->msg,0,RInfo->len);
	*(uint16 *)(RInfo->msg + size_SerialNum) = RInfo->len - size_SerialNum;
	*(char *)(RInfo->msg + size_SerialNum + size_Length) = CMD_REPORT;
	memcpy(RInfo->msg + size_SerialNum + size_Length + size_Type,self_account,strlen(self_account));
	getCheckSum(RInfo->msg,RInfo->len);

	return RInfo;
}

//机房服务器定时report自己的ip和端口信息
void *pollReportInfo(void *arg)
{
	ReportInfo *ri = (ReportInfo *)arg;
	
	while(1)
	{
		//debug("into poll report Info");
		InSendQueue(ri->sockfd,&(ri->agentaddr),ri->msg,ri->len);
		//debug("send to %s, port %d\n",inet_ntoa(ri->agentaddr.sin_addr),ntohs(ri->agentaddr.sin_port));
		//my_sendto(ri->sockfd,ri->msg,ri->len,0,(struct sockaddr *)&(ri->agentaddr),sizeof(struct sockaddr));
		sleep(ReportPeriod);
	}
}
