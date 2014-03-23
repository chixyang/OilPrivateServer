
#include "private_resend.h"

#define OverTimePollTime 5  //循环简称列表上是否有节点发送完成，单位秒

//重发列表
ReSendNode *ReSendList = NULL;
//互斥访问量
pthread_mutex_t resend_lock = PTHREAD_MUTEX_INITIALIZER;

//加入超时重发列表
int InTimelyReSendQueue(int fd,struct sockaddr_in *addr,char *buf,uint16 len,enum ReSendType type,uint8 drum)
{
	if((fd == 0) || (addr == NULL) || (buf == NULL) || (len == (uint16)0) || (type == RESEND_NONE))
		return -1;
	
	//查询列表中是否已有该信息
	int size_SerialNum = 8;
	int size_CheckSum = 1;   //协议头和校验位不用比较，有可能不相同
	if(my_mutex_lock(&resend_lock))
		return -1;
	ReSendNode *node = ReSendList;
	while(node != NULL)
	{
		if((fd == node->sockfd) && (addr->sin_addr.s_addr == node->sendaddr.sin_addr.s_addr) && (addr->sin_port == node->sendaddr.sin_port) && (!memcmp(buf+size_SerialNum,node->buf+size_SerialNum,len-size_SerialNum-size_CheckSum)))
		{   //找到节点，直接返回，不用再添加
			pthread_mutex_unlock(&resend_lock);
			return 0;
		}
		node = node->next;
	}
	pthread_mutex_unlock(&resend_lock);
	
	//新建节点
	ReSendNode *rsn = (ReSendNode *)malloc(sizeof(ReSendNode));
	memset(rsn,0,sizeof(ReSendNode));
	rsn->sockfd = fd;			//套接字描述符
	memcpy(&(rsn->sendaddr),addr,sizeof(struct sockaddr_in));
	rsn->buf = (char *)malloc(len * sizeof(char));     //给发送数据分配空间
	memcpy(rsn->buf,buf,len);
	rsn->buflen = len;
	rsn->timer = MAX_TIME;
	rsn->type = type;
	rsn->drum = drum;
	pthread_mutex_init(&(rsn->lock),NULL);
	
	//将节点添加到发送列表中
	if(my_mutex_lock(&resend_lock))
	{
		pthread_mutex_destroy(&(rsn->lock));
		free(rsn);
		return -1;
	}
	if(ReSendList == NULL)
		ReSendList = rsn;
	else
	{
		ReSendNode *tmp = ReSendList;
		while(tmp->next)   //添加作为最后一个节点
			tmp = tmp->next;
		tmp->next = rsn;
	}
	pthread_mutex_unlock(&resend_lock);
	
	pthread_t tid;
	pthread_create(&tid,NULL,ReSendAction,(void *)rsn);   //创建线程循环执行重发命令
	rsn->tid = tid;
//	pthread_detach(tid);		//不要设置线程分离,因为要等待该线程完成，设置可能会出错
	
	return 0;
}

//节点退出重发列表，不再重发信息,当type为RESEND_HOLE时，drum为0，其他时候drum不为0
int QuitReSendQueue(struct sockaddr_in *addr,enum ReSendType type,uint8 drum)
{
	if((addr == NULL) || (type == RESEND_NONE))
		return -1;
	
	//修改resend列表，使timer为0，以达到取消发送的目的
	switch(type)
	{
		case RESEND_HOLE:
		{
			if(my_mutex_lock(&resend_lock))
				return -1;
			ReSendNode *rsn = ReSendList;
			while(rsn != NULL)
			{
				//仅比较ip和port是否相同
				if((rsn->sendaddr.sin_addr.s_addr == addr->sin_addr.s_addr) && (rsn->sendaddr.sin_port == addr->sin_port))
						break;
				rsn = rsn->next;
			}
			pthread_mutex_unlock(&resend_lock);
			//修改查询到的节点的timer值		
			if(rsn != NULL)
			{
				//申请节点操作锁
				if(my_mutex_lock(&(rsn->lock)))
				{
					rsn->timer = 0; //强制转为0
					return 0;//成功
				}
				rsn->timer = 0;  
				pthread_mutex_unlock(&(rsn->lock));
			}
		}
			break;
		case RESEND_ALARM:
		case RESEND_LEVEL_NODEON:
		case RESEND_LEVEL_NODEOFF:
		case RESEND_TEMP_NODEON:
		case RESEND_TEMP_NODEOFF:
		{
			if(my_mutex_lock(&resend_lock))
				return -1;
			ReSendNode *rsn = ReSendList;
			while(rsn != NULL)
			{
				//比较ip、port、type和drum
				if((rsn->sendaddr.sin_addr.s_addr == addr->sin_addr.s_addr) && (rsn->sendaddr.sin_port == addr->sin_port) && (rsn->type == type) && (rsn->drum == drum))
						break;
				rsn = rsn->next;
			}
			pthread_mutex_unlock(&resend_lock);
			//修改查询到的节点的timer值	
			if(rsn != NULL)
			{
				//申请节点操作锁
				if(my_mutex_lock(&(rsn->lock)))
				{
					rsn->timer = 0;
					return 0;
				}
				rsn->timer = 0; 
				pthread_mutex_unlock(&(rsn->lock));
			}
		}
			break;
		default:
			return -1;
	}
	
	return 0;	
}


//reSendAction函数
void *ReSendAction(void *arg)
{
	ReSendNode *rsn = (ReSendNode *)arg;
	while(1)
	{
		if(my_mutex_lock(&(rsn->lock)))
		{
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = PERIOD;
			select(0,NULL,NULL,NULL,&tv);
			continue;
		}
		if(rsn->timer <= 0) //重发任务结束
		{
			pthread_mutex_unlock(&rsn->lock);
			break;
		}
		rsn->timer--;
		pthread_mutex_unlock(&rsn->lock);
		InSendQueue(rsn->sockfd,&(rsn->sendaddr),rsn->buf,rsn->buflen); //放在外面没有问题，这些都不是会改变的变量
		//休眠PERIOD时间
		//debug("resend type : %x  resend timer : %d\n",rsn->type,rsn->timer);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = PERIOD;
		select(0,NULL,NULL,NULL,&tv);  //作定时使用
	}
	
	pthread_exit("resend done");
}

void *pollOverTimeReSendNode()
{
	while(1)
	{
		if(my_mutex_lock(&resend_lock))
		{
			sleep(OverTimePollTime);
			continue;
		}
		ReSendNode *rsn = ReSendList, *preRsn = NULL;
		
		while(rsn)
		{
			if(my_mutex_lock(&(rsn->lock)))
			{
				preRsn = rsn;
				rsn = preRsn->next;
				continue;
			}
			if(rsn -> timer <= 0) //需要从列表上删除该节点
			{
				if(rsn == ReSendList) //第一个节点
					ReSendList = rsn->next;
				else
					preRsn->next = rsn->next;
				
				pthread_mutex_unlock(&(rsn->lock));
				void *status;
				pthread_join(rsn->tid,&status); //等待创建的线程结束
				ReSendNode *tmp = rsn->next;
				//线程结束后释放该节点信息
				pthread_mutex_destroy(&rsn->lock);
				free(rsn->buf);
				free(rsn);
				rsn = tmp;
				continue;
			}
			pthread_mutex_unlock(&(rsn->lock));
			preRsn = rsn;
			rsn = preRsn->next;
		}
		pthread_mutex_unlock(&resend_lock);

		sleep(OverTimePollTime);
	}
}
