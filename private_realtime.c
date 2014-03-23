
//实时信息的数据结构和处理函数
#include "private_realtime.h"

#define OverTimeRTPollTime 180  //超时节点轮询删除时间，三分钟，单位秒

//油桶数量
const uint8 NumofDrums = 6;

RealSendNode *RealSendList = NULL;
pthread_mutex_t send_lock = PTHREAD_MUTEX_INITIALIZER;

//前第一级记数的液位值
uint16 *former_first_level;

//前第二级记数的液位值
uint16 *former_second_level;

//前第三级记数的液位值
uint16 *former_third_level;                 //以上数据只有单线程操作，所以不用加锁

//第一级液位节点列表
LevelFirstNode *LevelFirstList = NULL;
//第二级液位节点列表
LevelSecondNode *LevelSecondList = NULL;
//第三级液位节点列表
LevelThirdNode *LevelThirdList = NULL;

//第一级温度节点列表
TempFirstNode *TempFirstList = NULL;
//第二级液位节点列表
TempSecondNode *TempSecondList = NULL;
//第三级液位节点列表
TempThirdNode *TempThirdList = NULL;

//初始化实时信息结构列表
void initRealLTList()
{
	//初始化实时发送列表
	RealSendList = NULL;
	//初始化计数器和液位数据和
	former_first_level = (uint16 *)malloc(sizeof(uint16) * NumofDrums);
	memset(former_first_level,0,sizeof(uint16) * NumofDrums);
	former_second_level= (uint16 *)malloc(sizeof(uint16) * NumofDrums);
	memset(former_second_level,0,sizeof(uint16) * NumofDrums);
	former_third_level = (uint16 *)malloc(sizeof(uint16) * NumofDrums);
	memset(former_third_level,0,sizeof(uint16) * NumofDrums);
	
	//首节点
	LevelFirstList = (LevelFirstNode *)malloc(sizeof(LevelFirstNode));
	memset(LevelFirstList,0,sizeof(LevelFirstNode));
	LevelFirstList->drum = 1;								//桶号从1开始
	pthread_mutex_init(&(LevelFirstList->lock),NULL);		//初始化锁
	LevelFirstNode *lf = LevelFirstList;
	//首节点
	LevelSecondList = (LevelSecondNode *)malloc(sizeof(LevelSecondNode));
	memset(LevelSecondList,0,sizeof(LevelSecondNode));
	LevelSecondList->drum = 1;
	pthread_mutex_init(&(LevelSecondList->lock),NULL);
	LevelSecondNode *ls = LevelSecondList;
	//首节点
	LevelThirdList = (LevelThirdNode *)malloc(sizeof(LevelThirdNode));
	memset(LevelThirdList,0,sizeof(LevelThirdNode));
	LevelThirdList->drum = 1;
	pthread_mutex_init(&(LevelThirdList->lock),NULL);
	LevelThirdNode *lt = LevelThirdList;
	//首节点
	TempFirstList = (TempFirstNode *)malloc(sizeof(TempFirstNode));
	memset(TempFirstList,0,sizeof(TempFirstNode));
	TempFirstList->drum = 1;
	pthread_mutex_init(&(TempFirstList->lock),NULL);
	TempFirstNode *tf = TempFirstList;
	//首节点
	TempSecondList = (TempSecondNode *)malloc(sizeof(TempSecondNode));
	memset(TempSecondList,0,sizeof(TempSecondNode));
	TempSecondList->drum = 1;
	pthread_mutex_init(&(TempSecondList->lock),NULL);
	TempSecondNode *ts = TempSecondList;
	//首节点
	TempThirdList = (TempThirdNode *)malloc(sizeof(TempThirdNode));
	memset(TempThirdList,0,sizeof(TempThirdNode));
	TempThirdList->drum = 1;
	pthread_mutex_init(&(TempThirdList->lock),NULL);
	TempThirdNode *tt = TempThirdList;
	//循环初始化其他节点
	for(int i = 1;i < NumofDrums;i++)
	{
		lf->next = (LevelFirstNode *)malloc(sizeof(LevelFirstNode));
		lf = lf->next;
		memset(lf,0,sizeof(LevelFirstNode));
		lf->drum = i + 1;
		pthread_mutex_init(&(lf->lock),NULL);
	
		ls->next = (LevelSecondNode *)malloc(sizeof(LevelSecondNode));
		ls = ls->next;
		memset(ls,0,sizeof(LevelSecondNode));
		ls->drum = i + 1;
		pthread_mutex_init(&(ls->lock),NULL);
	
		lt->next = (LevelThirdNode *)malloc(sizeof(LevelThirdNode));
		lt = lt->next;
		memset(lt,0,sizeof(LevelThirdNode));
		lt->drum = i + 1;
		pthread_mutex_init(&(lt->lock),NULL);
	
		tf->next = (TempFirstNode *)malloc(sizeof(TempFirstNode));
		tf = tf->next;
		memset(tf,0,sizeof(TempFirstNode));
		tf->drum = i + 1;
		pthread_mutex_init(&(tf->lock),NULL);
	
		ts->next = (TempSecondNode *)malloc(sizeof(TempSecondNode));
		ts = ts->next;
		memset(ts,0,sizeof(TempSecondNode));
		ts->drum = i + 1;
		pthread_mutex_init(&(ts->lock),NULL);
	
		tt->next = (TempThirdNode *)malloc(sizeof(TempThirdNode));
		tt = tt->next;
		memset(tt,0,sizeof(TempThirdNode));
		tt->drum = i + 1;
		pthread_mutex_init(&(tt->lock),NULL);
	}
}

//收到新数据信息,数据取地址，type只有两种first类型
int addNewLTInfo(enum LTLabel type,void *info,uint8 drum)
{
	if((info == NULL) || (drum == (uint8)0) || (drum >= NumofDrums))
		return -1;
	
	switch(type)
	{
		case L_FIRST: //收到液位信息
		{	LevelFirstNode *lf = LevelFirstList;
			int step = 0;  //用于直接到达二级、三级节点结构
			while(lf != NULL)
			{
				if(lf->drum == drum)  //找到添加的节点，这些节点都不会被删除，并且drum不会改变，所以，锁lock可以不用包含这句话
				{	
					if(my_mutex_lock(&lf->lock)) //这里就需要加锁，未获取到锁
						return -1;
					uint64 curtime = getCurrentTimeSeconds();//获取当前时间
					uint16 level = *(uint16 *)info;  //获取液位信息
					LTInfo *ltinfo = NULL;
					int alarm = 0;
					uint16 first_leveldiff = 0;
					if(lf->time != (uint64)0)  //如果不为系统第一次接收数据
					{
						//判断是否需要报警
						if((former_first_level[drum - 1] > level) && ((first_leveldiff=(former_first_level[drum - 1] - level)) >= LevelFirstDiff))
						{
							//debug("first level alarm\n");
							//报警
							sendAlarmBroadInfo(&curtime,REAL_ALARM_FISRT,drum,first_leveldiff);
							alarm = 1;
						}
						
						//不是系统第一次接受数据的话需要将原来的值存入数据库中
						ltinfo = (LTInfo *)malloc(sizeof(LTInfo));
						memset(ltinfo,0,sizeof(LTInfo));
						ltinfo->time = lf->time;
						ltinfo->drum = lf->drum;
						ltinfo->lt.level = lf->lt.level;
						if(addLTInfo(ltinfo,L_FIRST))
						{
							debug("add ltinfo first level error\n");
							free(ltinfo);
							pthread_mutex_unlock(&lf->lock);
							return -1;
						}
					}
					former_first_level[drum - 1] = level;
					//开始修改第一级节点信息
					lf->time = curtime;
					lf->lt.level = level;
					if(lf->active == 0)
					{
						lf->active = 1;
						/******向客户端发送信息通知节点成功接入协议，轮询实时发送列表，然后发送*******/
						//debug("******node add in success*********\n");
						sendNodeOnOffBroadInfo(&curtime,REAL_LEVEL_NODE_ON,drum);
					}
					
					pthread_mutex_unlock(&(lf->lock));
					lf = NULL;  //以防出错
					
					/**接着找到第二级节点并修改信息**/
					LevelSecondNode *ls = LevelSecondList;
					for(int i = 0; i < step; i++)  //根据初始化时的结构可知，各级的桶号是一一对应的
						ls = ls->next;
					if((ls == NULL) || (ls->drum != drum))
					{
						debug("can not find second level by first\n");
						free(ltinfo);
						return -1;
					}
					//找到第二级节点，比较时间，添加节点信息
					if(my_mutex_lock(&(ls->lock)))
					{
						free(ltinfo);
						return -1;
					}
					if((curtime / FSTTOSEC) == (ls->time / FSTTOSEC))  //在同一个时间段内,time和ls->time除以每小时的秒数值肯定相同
					{
						//时间time不能修改
						ls->lt.sum_level += level;
						ls->num++;
						pthread_mutex_unlock(&(ls->lock));
					}
					else				//不在同一个时间段内
					{
						if(ls->num != 0)   //非第一次存入值,第一次的时候num也为0
						{
							uint16 second_leveldiff = 0;
							//判断是否需要报警
							if((!alarm) && (former_second_level[drum-1] > level) && ((second_leveldiff=(former_second_level[drum-1] - level)) >= LevelSecondDiff))
							{
								debug("second level alarm\n");
								//报警
								sendAlarmBroadInfo(&curtime,REAL_ALARM_SECOND,drum,second_leveldiff);
								alarm = 1;
							}
							
							//先将原时间段存入数据库中
							ltinfo->time = ls->time;					//drum前面已经赋值过了
							ltinfo->lt.level = (uint16)((ls->lt.sum_level) / (ls->num));    //存入数据库的是平均值
							ltinfo->num = ls->num;
							if(addLTInfo(ltinfo,L_SECOND))
							{
								pthread_mutex_unlock(&(ls->lock));
								debug("add ltinfo error");
								free(ltinfo);
								return -1;
							}
						}
						former_second_level[drum-1] = level;
						//然后给该二级节点赋新值
						uint32 sum_level = ls->lt.sum_level;
						uint16 num = ls->num;
						ls->time = curtime;
						ls->lt.sum_level = level;
						ls->num = 1;
						pthread_mutex_unlock(&(ls->lock));
						ls = NULL;
						
						/** 当二级节点被存入数据库后，同时将该节点加入第三级节点中 **/
						LevelThirdNode *lt = LevelThirdList;
						for(int i = 0; i < step; i++)  //找到三级节点
							lt = lt->next;
						if((lt == NULL) || (lt->drum != drum))
						{
							debug("can not find third level by second\n");
							free(ltinfo);
							return -1;
						}
						if(my_mutex_lock(&(lt->lock)))
						{
							free(ltinfo);
							return -1;
						}
						if((curtime / SECTOTHD) == (lt->time / SECTOTHD)) //与第三级节点在同一个设定的时间段内
						{
							lt->lt.sum_level += sum_level;
							lt->num += num;
							pthread_mutex_unlock(&(lt->lock));
						}
						else    //不在一个时间段内
						{
							if(lt->num != 0)
							{
								uint16 third_leveldiff = 0;
								//判断是否需要报警
								if((!alarm) && (former_third_level[drum-1] > level) && ((third_leveldiff=(former_third_level[drum-1] - level)) >= LevelThirdDiff))
								{//报警
									debug("third level alarm\n");
									sendAlarmBroadInfo(&curtime,REAL_ALARM_THIRD,drum,third_leveldiff);
								}
								ltinfo->time = lt->time;					//drum前面已经赋值过了
								ltinfo->lt.level = (uint16)((lt->lt.sum_level) / (lt->num));    //存入数据库的是平均值
								ltinfo->num = lt->num;
								if(addLTInfo(ltinfo,L_THIRD))
								{
									pthread_mutex_unlock(&(lt->lock));
									debug("add ltinfo error");
									free(ltinfo);
									return -1;
								}
							}
							former_third_level[drum-1] = level;
							//给三级节点赋新值
							lt->time = curtime;
							lt->lt.sum_level = sum_level;
							lt->num = num;
							pthread_mutex_unlock(&(lt->lock));
						}
					}
					if(ltinfo)
						free(ltinfo);
					return 0;
				}
				lf = lf->next;
				step++;
			}
		}
			break;
		case T_FIRST: //收到温度信息
		{//	debug("in temp first add\n");
			TempFirstNode *tf = TempFirstList;
			int step = 0;  //用于直接到达二级、三级节点结构
			while(tf != NULL)
			{
				if(tf->drum == drum)  //找到添加的节点，这些节点都不会被删除，并且drum不会改变，所以，锁lock可以不用包含这句话
				{
					if(my_mutex_lock(&(tf->lock)))
						return -1;
					uint64 curtime = getCurrentTimeSeconds(); //获取当前时间
					float temp = *(float *)info;  //获取温度信息
					LTInfo *ltinfo = NULL;
					if(tf->time != (uint64)0)  //如果不为系统第一次接收数据
					{
						//温度不需要报警，将原来的值存入数据库中
						ltinfo = (LTInfo *)malloc(sizeof(LTInfo));
						memset(ltinfo,0,sizeof(LTInfo));
						ltinfo->time = tf->time;
						ltinfo->drum = tf->drum;
						ltinfo->lt.temp = tf->lt.temp;
						if(addLTInfo(ltinfo,T_FIRST))
						{
							debug("add ltinfo error");
							free(ltinfo);
							return -1;
						}
					}
					//开始修改第一级节点信息
					tf->time = curtime;
					tf->lt.temp = temp;
					if(tf->active == 0)
					{
						tf->active = 1;
						/******向客户端发送信息通知节点成功接入协议，轮询实时发送列表，然后发送*******/
						sendNodeOnOffBroadInfo(&curtime,REAL_TEMP_NODE_ON,drum);
					}
					pthread_mutex_unlock(&(tf->lock));
					tf = NULL;  //以防出错
					
					/**接着找到第二级节点并修改信息**/
					TempSecondNode *ts = TempSecondList;
					for(int i = 0; i < step; i++)  //根据初始化时的结构可知，各级的桶号是一一对应的
						ts = ts->next;
					if((ts == NULL) || (ts->drum != drum))
					{
						debug("can not find second level by first\n");
						free(ltinfo);
						return -1;
					}
					//找到第二级节点，比较时间，添加节点信息
					if(my_mutex_lock(&(ts->lock)))
					{
						free(ltinfo);
						return -1;
					}
					if((curtime / FSTTOSEC) ==(ts->time / FSTTOSEC))  //在同一个时间段内,time一定比ts->time大，第一次的话ts->time 为0，差值一定大于FSTTOSEC，所以这几句代码不执行
					{
						//时间time不能修改
						ts->lt.sum_temp += temp;
						ts->num++;
						pthread_mutex_unlock(&(ts->lock));
					}
					else				//不在同一个时间段内
					{
						if(ts->num != 0)
						{
							//先将原时间段存入数据库中
							ltinfo->time = ts->time;					//drum前面已经赋值过了
							ltinfo->lt.temp = (ts->lt.sum_temp) / (ts->num);    //存入数据库的是平均值
							ltinfo->num = ts->num;
							if(addLTInfo(ltinfo,T_SECOND))
							{
								pthread_mutex_unlock(&(ts->lock));
								debug("add ltinfo error");
								free(ltinfo);
								return -1;
							}
						}
						//然后给该二级节点赋新值
						float sum_temp = ts->lt.sum_temp;
						uint16 num = ts->num;
						ts->time = curtime;
						ts->lt.sum_temp = temp;
						ts->num = 1;
						pthread_mutex_unlock(&(ts->lock));
						ts = NULL;
						
						/** 当二级节点被存入数据库后，同时将该节点加入第三级节点中 **/
						TempThirdNode *tt = TempThirdList;
						for(int i = 0; i < step; i++)  //找到三级节点
							tt = tt->next;
						if((tt == NULL) || (tt->drum != drum))
						{
							debug("can not find third level by second\n");
							free(ltinfo);
							return -1;
						}
						if(my_mutex_lock(&(tt->lock)))
						{
							free(ltinfo);
							return -1;
						}
						if((curtime / SECTOTHD) ==(tt->time / SECTOTHD)) //与第三级节点在同一个设定的时间段内
						{
							tt->lt.sum_temp += sum_temp;
							tt->num += num;
							pthread_mutex_unlock(&(tt->lock));
						}
						else    //不在一个时间段内
						{
							if(tt->num != 0)
							{
								ltinfo->time = tt->time;					//drum前面已经赋值过了
								ltinfo->lt.temp = (tt->lt.sum_temp) / (tt->num);    //存入数据库的是平均值
								ltinfo->num = tt->num;
								if(addLTInfo(ltinfo,T_THIRD))
								{
									pthread_mutex_unlock(&(tt->lock));
									debug("add ltinfo error");
									free(ltinfo);
									return -1;
								}
							}
							//给三级节点赋新值
							tt->time = curtime;
							tt->lt.sum_temp = sum_temp;
							tt->num = num;
							pthread_mutex_unlock(&(tt->lock));
						}
					}
					if(ltinfo)  //不为空则释放
						free(ltinfo);
					return 0;
				}
				tf = tf->next;
				step++;
			}
		}
			break;
		default:
			return -1;
	}
	
	return -1;
}


//查询实时数据信息,当drum为0时，表示全部查询，查询结果仅为first 第一级内容，其他级内容不为实时内容
RealInfo *getCurrentLTInfo(uint8 drum)
{
	//debug("in get current lt info\n");
	LevelFirstNode *lf = LevelFirstList;
	TempFirstNode *tf = TempFirstList;
	RealInfo *ri = NULL;
	RealInfo *node = NULL;
	
	//debug("drum = %d\n",drum);
	if(drum == 0) 
	{//全部查询

		while((lf != NULL) && (tf != NULL))
		{
			if(my_mutex_lock(&(lf->lock)))
			{
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(my_mutex_lock(&(tf->lock)))
			{
				pthread_mutex_unlock(&(lf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			//debug("lf->drum : %d, lf->active : %d; tf->drum : %d,tf->active : %d\n",lf->drum,lf->active,tf->drum,tf->active);
			if(!((lf->active) || (tf->active)))	//至少保证单个桶上有一个节点在工作，否则不作为查询结果
			{
				pthread_mutex_unlock(&(lf->lock));   //锁嵌套是非常危险的行为，但是所有代码中只有此处用到了锁嵌套，所以安全
				pthread_mutex_unlock(&(tf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(ri != NULL)
			{
				node->next = (RealInfo *)malloc(sizeof(RealInfo));
				node = node->next;
				memset(node,0,sizeof(RealInfo));
			}
			else
			{
				ri = (RealInfo *)malloc(sizeof(RealInfo));
				memset(ri,0,sizeof(RealInfo));
				node = ri;
			}
			node->drum = lf->drum;
			node->level = lf->lt.level;
			if(lf->active == 0)
				node->level = 0;
			node->temp = tf->lt.temp;
			if(tf->active == 0)
				node->temp == .0;
			
			pthread_mutex_unlock(&(tf->lock));
			pthread_mutex_unlock(&(lf->lock));
			//lf与tf对应的
			lf = lf->next;
			tf = tf->next;
		}

		//debug("geted real info\n");
		return ri;
	}
	else  //单个查询
	{
		while((lf != NULL) && (tf != NULL))
		{
			if(my_mutex_lock(&(lf->lock)))
			{
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(my_mutex_lock(&(tf->lock)))
			{
				pthread_mutex_unlock(&(lf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(!((lf->active) || (tf->active)))
			{
				pthread_mutex_unlock(&(tf->lock));
				pthread_mutex_unlock(&(lf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if((lf->drum == drum) && (tf->drum == drum))
			{
				ri = (RealInfo *)malloc(sizeof(RealInfo));
				memset(ri,0,sizeof(RealInfo));
				ri->drum = drum;
				ri->level = lf->lt.level;
				if(lf->active == 0)
					ri->level = 0;
				ri->temp = tf->lt.temp;
				if(tf->active == 0)
					ri->temp == .0;

				pthread_mutex_unlock(&(tf->lock));
				pthread_mutex_unlock(&(lf->lock));
				break;
			}
			
			pthread_mutex_unlock(&(tf->lock));
			pthread_mutex_unlock(&(lf->lock));
			lf = lf->next;
			tf = tf->next;
		}
		//debug("get single realinfo\n");
		return ri;
	}
}

//释放RealInfo列表
void freeRealInfoList(RealInfo *ri)
{
	if(ri != NULL)
	{
		RealInfo *tmp = ri->next;
		while(tmp != NULL)
		{
			ri->next = tmp->next;
			free(tmp);
			tmp = ri->next;
		}
		free(ri);
	}
}

//轮询列表(查看是否有节点超时未收到数据),线程函数
void *pollDiedNode()
{
	while(1)
	{
		sleep(PollDiedTime);

		uint64 time = getCurrentTimeSeconds();
		//检查第一级液位节点
		LevelFirstNode *lf = LevelFirstList;
		while(lf != NULL)
		{
			if(my_mutex_lock(&(lf->lock)))
			{
				lf = lf->next;
				continue;
			}
			if((lf->active == 1) && ((time - lf->time) >= ActiveDiff))  //节点已经died
			{
				lf->active = 0;
				/**向客户端发送信息通知该节点已经died**/
				sendNodeOnOffBroadInfo(&time,REAL_LEVEL_NODE_OFF,lf->drum);
			}
			pthread_mutex_unlock(&(lf->lock));
			lf = lf->next;
		}
		//检查第一级温度节点
		TempFirstNode *tf = TempFirstList;
		while(tf != NULL)
		{
			if(my_mutex_lock(&(tf->lock)))
			{
				tf = tf->next;
				continue;
			}
			if((tf->active == 1) && ((time - tf->time) >= ActiveDiff))  //节点已经died
			{
				tf->active = 0;
				/**向客户端发送信息通知该节点已经died**/
				sendNodeOnOffBroadInfo(&time,REAL_TEMP_NODE_OFF,tf->drum);
			}
			pthread_mutex_unlock(&(tf->lock));
			tf = tf->next;
		}
	}
}

//加入实时发送列表
int InRealSendList(char *username,uint8 drum,int fd,struct sockaddr_in *addr, enum RealLabel type)
{
	if((username == NULL) || (fd == 0) || (addr == NULL) || (type == REAL_NONE))
		return -1;
	
	//先判断该节点是否已经存在于列表中
	if(my_mutex_lock(&(send_lock)))
		return -1;
	RealSendNode *node = RealSendList, *preNode = NULL;
	while(node != NULL)
	{
		if(!memcmp(node->username,username,strlen(username))) //已经存在
		{
			//检查网络地址是否相同，也相同的话则不再建立新节点，不同则删除当前节点并建立新节点
			if((node->sendaddr.sin_addr.s_addr == addr->sin_addr.s_addr) && (node->sendaddr.sin_port == addr->sin_port))
			{	debug("duplicate node in real send list\n");
				pthread_mutex_unlock(&(send_lock));
				RTSendAction(node);
				return 0;
			}
			else
			{
				//删除原来节点,将该节点从列表上删除并且设置原来节点的timer值，线程会自动free该节点
				if(node == RealSendList) //要删除的是头结点
					RealSendList = node->next;
				else //普通节点
					preNode->next = node->next;
			}
			break;
		}
		
		preNode = node;
		node = preNode->next;
	}
	pthread_mutex_unlock(&(send_lock));
	//设置timer标志，使线程删除该节点，放在外面避免了锁嵌套
	if(node != NULL)
	{
		if(my_mutex_lock(&(node->lock)))
		{
			node->timer = 0;   //即使未获取到锁也要强制转换，因为该节点已经删除
			return -1;
		}
		node->timer = 0;
		pthread_mutex_unlock(&(node->lock));
	}
	
	//新建节点(节点未找到或者刚刚被删除)
	RealSendNode *rsn = (RealSendNode *)malloc(sizeof(RealSendNode));
	memset(rsn,0,sizeof(RealSendNode));
	memcpy(rsn->username,username,strlen(username));  //用户名赋值
	rsn->drum = drum;
	rsn->fd = fd;				//套接字文件
	memcpy(&(rsn->sendaddr),addr,sizeof(struct sockaddr_in));  //用户地址
	rsn->type = type;				//请求类型
	rsn->timer = REAL_SEND_TIMER;
	pthread_mutex_init(&(rsn->lock),NULL);
	
	//debug("before add in real send list\n");
	//加入列表
	if(my_mutex_lock(&(send_lock)))
	{
		pthread_mutex_destroy(&(rsn->lock));
		free(rsn);
		return -1;
	}
	if(RealSendList == NULL)  //列表起始为空
		RealSendList = rsn;
	else   //添加作为头结点
	{
		rsn->next = RealSendList;
		RealSendList = rsn;
	}
	pthread_mutex_unlock(&(send_lock));
	
	//给一个线程执行该节点任务(不需要了)
//	pthread_t tid;
//	pthread_create(&tid,NULL,RTSendAction,(void *)rsn);   //创建线程循环执行重发命令
//	rsn->tid = tid;
	rsn->tid = 0;
	//pthread_detach(tid);		//不要设置线程分离，因为要等待该线程完成

	debug("after add in real time list\n");
	return 0;
}

//实时发送列表项必须要客户端发信息才会删除的
int QuitRealSendList(char *username,uint8 drum,enum RealLabel type)
{
	if((username == NULL))
		return -1;
	debug("in quit real send,drum : %d\n",drum);
	//找到节点
	if(my_mutex_lock(&(send_lock)))
		return -1;
	RealSendNode *rsn = RealSendList, *preRsn = NULL;
	while(rsn != NULL)
	{
		if((!memcmp(username,rsn->username,strlen(username))) && (drum == rsn->drum) && (type == rsn->type))
			break;
		preRsn = rsn;
		rsn = preRsn->next;
	}
	 
	if(rsn == NULL) //要么没找到，要么节点列表为空
	{
		pthread_mutex_unlock(&(send_lock));
		return 0;
	}
	else  //有可能是头结点,也有可能是普通节点
	{
		if(preRsn == NULL)   //头结点
			RealSendList = rsn->next;
		else
			preRsn->next = rsn->next;
	}
		
	pthread_mutex_unlock(&(send_lock));
	
	//设置rsn的timer为0，让线程自动free该节点
	if(my_mutex_lock(&(rsn->lock)))
	{
		rsn->timer = 0;  //强制转换，因为已经从列表删除
		return 0;  //表示已经成功
	}
	rsn->timer = 0;
	pthread_mutex_unlock(&(rsn->lock));
	
	return 0;
}

//线程函数，实时信息定时发送函数
void *RTSendAction(void *arg)
{
	RealSendNode *rsn = (RealSendNode *)arg;
	//while(1)
	{
		//debug("real time send poll once\n");
		if(my_mutex_lock(&(rsn->lock)))
		{
			debug("can not get lock\n");
	//		continue;
		}
		if(rsn->timer <= 0)
		{
			debug("timer <= 0\n");
			pthread_mutex_unlock(&(rsn->lock));
			sleep(REAL_SEND_PERIOD);
	//		continue;
		}
		RealInfo *ri = getCurrentLTInfo(rsn->drum);  //获取实时信息列表
		if(ri == NULL)
		{
			//debug("ri == NULL");
			pthread_mutex_unlock(&(rsn->lock));
	//		continue;
		}
		uint16 len = 0;
		char *buf = getRealLTInfo(ri,rsn->username,&len);   //获取发送协议
		freeRealInfoList(ri);
		if(buf == NULL)
		{
			debug("buf == NULL");
			pthread_mutex_unlock(&(rsn->lock));
	//		continue;
		}
	
	//	int sendlabel = rsn->timer;
	//	rsn->timer--;
	//	debug("rsn timer :%d\n",rsn->timer);
		pthread_mutex_unlock(&(rsn->lock));
		//因为自始至终都是一个线程操作rsn除了timer以外的东西，所以fd和sendaddr不会改变，这句代码放在外面没有问题
	//实时信息不再定时发送
	//	if(sendlabel == REAL_SEND_TIMER)
			InSendQueue(rsn->fd,&(rsn->sendaddr),buf,len);  //发送
		//debug("real send once ,length: %d  ret : %d\n",len,ret);
		free(buf);
	
	//	sleep(REAL_SEND_PERIOD); //休眠
	}
	
//	pthread_exit("real time send quit");
}

//轮询超时的实时节点，找到删除
void *pollOverTimeRTNode()
{
	while(1)
	{
		if(my_mutex_lock(&send_lock))
		{
			sleep(OverTimeRTPollTime);  //获取不到锁，继续休眠
			continue;
		}
		RealSendNode *rsn = RealSendList, *preRsn = NULL;
		
		while(rsn)
		{
			if(my_mutex_lock(&(rsn->lock)))
			{
				preRsn = rsn;
				rsn = preRsn->next;
				continue;
			}
			if(rsn->timer <= 0) //需要从列表上删除该节点
			{
				if(rsn == RealSendList) //第一个节点
					RealSendList = rsn->next;
				else
					preRsn->next = rsn->next;
				
				pthread_mutex_unlock(&(rsn->lock));
				//void *status;
				//pthread_join(rsn->tid,&status); //等待创建的线程结束
				RealSendNode *tmp = rsn->next;
				//线程结束后释放该节点信息
				pthread_mutex_destroy(&rsn->lock);
				free(rsn);
				rsn = tmp;
				continue;
			}
			pthread_mutex_unlock(&(rsn->lock));
			preRsn = rsn;
			rsn = preRsn->next;
		}
		pthread_mutex_unlock(&send_lock);

		sleep(OverTimeRTPollTime);
	}
}

//找到实时发送列表中的节点，并通过该列表将消息和发送信息加入到发送列表，type为reallabel枚举常量，目前取值范围为ALARM_FISRT，ALARM_SECOND，ALARM_THIRD，NODE_ON，NODE_OFF
int sendNodeOnOffBroadInfo(uint64 *time,enum RealLabel type,uint8 drum)
{
	//如果生成的协议为空
	if((time == NULL) || (drum == (uint8)0))
		return -1;
	
	char resend_type = RESEND_NONE;
	char *buf = NULL;
	uint16 len = 0;
	switch(type)
	{
		case REAL_LEVEL_NODE_ON:
			resend_type = RESEND_LEVEL_NODEON;
			break;
		case REAL_LEVEL_NODE_OFF:
			resend_type = RESEND_LEVEL_NODEOFF;
			break;
		case REAL_TEMP_NODE_ON:
			resend_type = RESEND_TEMP_NODEON;
			break;
		case REAL_TEMP_NODE_OFF:
			resend_type = RESEND_TEMP_NODEOFF;
			break;
		default:
			return -1;
	}
	if(my_mutex_lock(&send_lock))
		return -1;
	RealSendNode *rsn = RealSendList;
	//寻找节点发送
	while(rsn != NULL)
	{
		buf = getNodeOnOffReport(rsn->username,drum,time,type,&len);
		//所有节点都发送，并且要保证发送成功，即保证失败重发
		InTimelyReSendQueue(rsn->fd,&(rsn->sendaddr),buf,len,resend_type,drum);   //发送到定时发送函数，以确保这些重要信息完整的被接收
		free(buf);
		len = 0;
		rsn = rsn->next;
	}
	pthread_mutex_unlock(&send_lock);
	return 0;
}


//发送节点液位报警信息
int sendAlarmBroadInfo(uint64 *time,enum RealLabel type,uint8 drum,uint32 leveldiff)
{
	//如果生成的协议为空
	if((time == NULL) || (drum == (uint8)0) || (leveldiff == (uint32)0))
		return -1;
	
	char resend_type = RESEND_NONE;
	char *buf = NULL;
	uint16 len = 0;
	switch(type)
	{
		//以下信息都是广播信息
		case REAL_ALARM_FISRT:
		case REAL_ALARM_SECOND:
		case REAL_ALARM_THIRD:
			resend_type = RESEND_ALARM;
			break;
		default:
			return -1;
	}

	if(my_mutex_lock(&send_lock))
		return -1;
	RealSendNode *rsn = RealSendList;
	//寻找节点发送
	while(rsn != NULL)
	{
		buf = getAlarmInfo(rsn->username,drum,time,type,leveldiff,&len);
		//所有节点都发送，并且要保证发送成功，即保证失败重发
		InTimelyReSendQueue(rsn->fd,&(rsn->sendaddr),buf,len,resend_type,drum);   //发送到定时发送函数，以确保这些重要信息完整的被接收
		free(buf);
		len = 0;
		rsn = rsn->next;
	}
	pthread_mutex_unlock(&send_lock);
	return 0;
}



