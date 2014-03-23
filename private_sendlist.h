
#ifndef SENDLIST_H_
#define SENDLIST_H_
//�����б�������

#include "private_debug.h"
#include "private_sockio.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

//���ͽڵ�
struct send_node {
	int sockfd;
	struct sockaddr_in sendaddr;
	char *buf;
	uint16 buflen;
	struct send_node *next;
};
typedef struct send_node SendNode;

//�����б�
struct send_list{
	SendNode *head;   //��һ��ͷ�ڵ�
	uint16 node_num;
	pthread_mutex_t send_list_lock;
	pthread_cond_t send_list_cond;
};

extern struct send_list *SendList;

/**
 * ��ʼ�������б�
 */
 void initSendList();
 
 /**
  * ������м����½ڵ�
  * @param fd ��ǰʹ�õķ���������
  * @param addr Ŀ�ĵ�ַ
  * @param buf Ҫ���͵�����
  * @param len �������ݵĳ���
  * @return 0��ʾ����ɹ�������ʧ��
  */
int InSendQueue(int fd,struct sockaddr_in *addr,char *buf,uint16 len);

/**
 * �߳���ѯ���񣬵ȴ������͡�ɾ���ڵ�
 */
 void *pollSendList();


#endif
