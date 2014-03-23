
#include "private_resend.h"

#define OverTimePollTime 5  //ѭ������б����Ƿ��нڵ㷢����ɣ���λ��

//�ط��б�
ReSendNode *ReSendList = NULL;
//���������
pthread_mutex_t resend_lock = PTHREAD_MUTEX_INITIALIZER;

//���볬ʱ�ط��б�
int InTimelyReSendQueue(int fd,struct sockaddr_in *addr,char *buf,uint16 len,enum ReSendType type,uint8 drum)
{
	if((fd == 0) || (addr == NULL) || (buf == NULL) || (len == (uint16)0) || (type == RESEND_NONE))
		return -1;
	
	//��ѯ�б����Ƿ����и���Ϣ
	int size_SerialNum = 8;
	int size_CheckSum = 1;   //Э��ͷ��У��λ���ñȽϣ��п��ܲ���ͬ
	if(my_mutex_lock(&resend_lock))
		return -1;
	ReSendNode *node = ReSendList;
	while(node != NULL)
	{
		if((fd == node->sockfd) && (addr->sin_addr.s_addr == node->sendaddr.sin_addr.s_addr) && (addr->sin_port == node->sendaddr.sin_port) && (!memcmp(buf+size_SerialNum,node->buf+size_SerialNum,len-size_SerialNum-size_CheckSum)))
		{   //�ҵ��ڵ㣬ֱ�ӷ��أ����������
			pthread_mutex_unlock(&resend_lock);
			return 0;
		}
		node = node->next;
	}
	pthread_mutex_unlock(&resend_lock);
	
	//�½��ڵ�
	ReSendNode *rsn = (ReSendNode *)malloc(sizeof(ReSendNode));
	memset(rsn,0,sizeof(ReSendNode));
	rsn->sockfd = fd;			//�׽���������
	memcpy(&(rsn->sendaddr),addr,sizeof(struct sockaddr_in));
	rsn->buf = (char *)malloc(len * sizeof(char));     //���������ݷ���ռ�
	memcpy(rsn->buf,buf,len);
	rsn->buflen = len;
	rsn->timer = MAX_TIME;
	rsn->type = type;
	rsn->drum = drum;
	pthread_mutex_init(&(rsn->lock),NULL);
	
	//���ڵ���ӵ������б���
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
		while(tmp->next)   //�����Ϊ���һ���ڵ�
			tmp = tmp->next;
		tmp->next = rsn;
	}
	pthread_mutex_unlock(&resend_lock);
	
	pthread_t tid;
	pthread_create(&tid,NULL,ReSendAction,(void *)rsn);   //�����߳�ѭ��ִ���ط�����
	rsn->tid = tid;
//	pthread_detach(tid);		//��Ҫ�����̷߳���,��ΪҪ�ȴ����߳���ɣ����ÿ��ܻ����
	
	return 0;
}

//�ڵ��˳��ط��б������ط���Ϣ,��typeΪRESEND_HOLEʱ��drumΪ0������ʱ��drum��Ϊ0
int QuitReSendQueue(struct sockaddr_in *addr,enum ReSendType type,uint8 drum)
{
	if((addr == NULL) || (type == RESEND_NONE))
		return -1;
	
	//�޸�resend�б�ʹtimerΪ0���Դﵽȡ�����͵�Ŀ��
	switch(type)
	{
		case RESEND_HOLE:
		{
			if(my_mutex_lock(&resend_lock))
				return -1;
			ReSendNode *rsn = ReSendList;
			while(rsn != NULL)
			{
				//���Ƚ�ip��port�Ƿ���ͬ
				if((rsn->sendaddr.sin_addr.s_addr == addr->sin_addr.s_addr) && (rsn->sendaddr.sin_port == addr->sin_port))
						break;
				rsn = rsn->next;
			}
			pthread_mutex_unlock(&resend_lock);
			//�޸Ĳ�ѯ���Ľڵ��timerֵ		
			if(rsn != NULL)
			{
				//����ڵ������
				if(my_mutex_lock(&(rsn->lock)))
				{
					rsn->timer = 0; //ǿ��תΪ0
					return 0;//�ɹ�
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
				//�Ƚ�ip��port��type��drum
				if((rsn->sendaddr.sin_addr.s_addr == addr->sin_addr.s_addr) && (rsn->sendaddr.sin_port == addr->sin_port) && (rsn->type == type) && (rsn->drum == drum))
						break;
				rsn = rsn->next;
			}
			pthread_mutex_unlock(&resend_lock);
			//�޸Ĳ�ѯ���Ľڵ��timerֵ	
			if(rsn != NULL)
			{
				//����ڵ������
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


//reSendAction����
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
		if(rsn->timer <= 0) //�ط��������
		{
			pthread_mutex_unlock(&rsn->lock);
			break;
		}
		rsn->timer--;
		pthread_mutex_unlock(&rsn->lock);
		InSendQueue(rsn->sockfd,&(rsn->sendaddr),rsn->buf,rsn->buflen); //��������û�����⣬��Щ�����ǻ�ı�ı���
		//����PERIODʱ��
		//debug("resend type : %x  resend timer : %d\n",rsn->type,rsn->timer);
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = PERIOD;
		select(0,NULL,NULL,NULL,&tv);  //����ʱʹ��
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
			if(rsn -> timer <= 0) //��Ҫ���б���ɾ���ýڵ�
			{
				if(rsn == ReSendList) //��һ���ڵ�
					ReSendList = rsn->next;
				else
					preRsn->next = rsn->next;
				
				pthread_mutex_unlock(&(rsn->lock));
				void *status;
				pthread_join(rsn->tid,&status); //�ȴ��������߳̽���
				ReSendNode *tmp = rsn->next;
				//�߳̽������ͷŸýڵ���Ϣ
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
