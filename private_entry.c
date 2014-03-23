//���ļ�Ϊ�����������

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include "private_sockio.h"
#include "private_debug.h"
#include "private_dbio.h"
#include "private_dbpool.h"
#include "private_sendlist.h"
#include "private_threadtask.h"
#include "private_realtime.h"

#define port 3302  //�����˿�

#define max_db_con 50   //������ݿ�������


int main(int argc,char *argv[])
{
	//��ʼ�����ݿ��
	dbpool_init(max_db_con);
	//��ʼ�������б�
	initSendList();
	//��ʼ��ʵʱ�����б�
	initRealLTList();
	//������Ծ�б����߳�
	pthread_t tid;
	//���������б����߳�
	pthread_create(&tid,NULL,&pollSendList,NULL);
	//������ѯ�����ڵ��߳�
	pthread_create(&tid,NULL,&pollDiedNode,NULL);
	//��ѯ�ط��б�
	pthread_create(&tid,NULL,&pollOverTimeReSendNode,NULL);
	//��ѯ��ʱʵʱ�ڵ�
	pthread_create(&tid,NULL,&pollOverTimeRTNode,NULL);
	//�����������׽���
	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;   //ipv4
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port);
	//����udp����
	int listenfd = socket(AF_INET,SOCK_DGRAM,0);
	if(listenfd < 0)
	{
		perror("socket error");
		return -1;
	}
	//��ʼ����ʱ������Ϣ
//	ReportInfo *RInfo = initReportInfo(listenfd);
//	if(RInfo != NULL)
//		pthread_create(&tid,NULL,&pollReportInfo,RInfo);  //��ʱ����
	//bind
	if(bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr))< 0)
	{
		perror("bind error");
		return -1;
	}
	//��ʼ����ʱ������Ϣ
	ReportInfo *RInfo = initReportInfo(listenfd);
	if(RInfo != NULL)
		pthread_create(&tid,NULL,&pollReportInfo,RInfo);  //��ʱ����
	else
	{
		debug("poll report info error\n");
		exit(1);
	}

	fd_set readset;   //socket��
	while(1)
	{       
		FD_ZERO(&readset);  //���
		FD_SET(listenfd,&readset); //�����������
		//��·ת��
		int ret = select(listenfd+1,&readset,NULL,NULL,NULL);
		if(ret <= 0)
			continue;
		//����Ƿ�׼���ɹ�
		ret = FD_ISSET(listenfd,&readset);
		if(ret == 0)
			continue;
		//��ʼ������
		RecvCommand *rc = (RecvCommand *)malloc(sizeof(RecvCommand));
		memset(rc,0,sizeof(RecvCommand));
		rc->sockfd = listenfd;
		socklen_t fromlen;
		uint16 len = my_recvfrom(listenfd,rc->buf,0,(struct sockaddr*)&(rc->recv_addr),&fromlen);
		rc->buflen = len;
		
		for(int i = 0;i<len;i++)
			printf("%x ",rc->buf[i]);
		debug("recv end\n");

		//���߳�ִ������
		pthread_t tid;
		pthread_create(&tid,NULL,parseCommand,rc);
		pthread_detach(tid);
	}
}
