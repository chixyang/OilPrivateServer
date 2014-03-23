
#ifndef RESEND_H_
#define RESEND_H_

/**��ʱ������Ϣ�ṹ�뺯���Ķ����ļ�**/

#include "private_sendlist.h"
#include "private_debug.h"
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define PERIOD 200000				//����ʱ����,��λ΢��us��Ҳ���������ͻ��˻ظ����ʱ����������������ڸ��¼������δ�յ��ظ���Ϣ�����ط���Ϣ

#define MAX_TIME  5              //�ط���������

enum ReSendType{
	RESEND_NONE,
	RESEND_ALARM,		//	��������
	RESEND_HOLE,	    //������
	RESEND_LEVEL_NODEON,		//Һλ�ڵ��������
	RESEND_LEVEL_NODEOFF,		//Һλ�ڵ���ʧ����
	RESEND_TEMP_NODEON,		//Һλ�ڵ��������
	RESEND_TEMP_NODEOFF		//Һλ�ڵ���ʧ����
};

//����б��������ip��port��type��drum
typedef struct resend_node{
	int sockfd;
	struct sockaddr_in sendaddr;
	char *buf;
	uint16 buflen;
	uint8 timer;
	enum ReSendType type;					//�ط����ͣ��������������ط����ͣ��������򶴡��ڵ���ֺ���ʧ
	uint8 drum;
	pthread_t tid;
	pthread_mutex_t lock;
	struct resend_node *next;
}ReSendNode;

//�ط��б�
extern ReSendNode *ReSendList;

/**
 * ���볬ʱ�ط��б�
 * @param fd �׽���������
 * @param addr Ҫ���͵������ַ
 * @param buf Ҫ���͵�Э������
 * @param len Ҫ���͵�Э�鳤��
 * @param type Ҫ���͵���Ϣ����RESEND_ALARM��RESEND_HOLE��RESEND_NODEON��RESEND_NODEOFF������
 * @param drum Ҫ���͵���Ϣ��Ӧ��Ͱ��
 * @return 0 ����ɹ�������ʧ��
 */
int InTimelyReSendQueue(int fd,struct sockaddr_in *addr,char *buf,uint16 len,enum ReSendType type,uint8 drum);

/**
 * �ڵ��˳���ʱ�ط��б������ط���Ϣ,��typeΪRESEND_HOLEʱ��drumΪ0������ʱ��drum��Ϊ0
 * @param addr ��Ϣ��Դ�������ַ
 * @param type ��Ϣ����RESEND_ALARM��RESEND_HOLE��RESEND_NODEON��RESEND_NODEOFF������
 * @param drum Ͱ�ţ���typeΪRESEND_HOLEʱ��drumΪ0������ʱ�̲�����Ϊ0
 * @return 0 ��ʾ�˳��ɹ�������ʧ��
 */
int QuitReSendQueue(struct sockaddr_in *addr,enum ReSendType type,uint8 drum);

/**
 * ��ɳ�ʱ�ط����̵��̺߳�����ÿ���̶߳�Ӧһ��ReSendNode����ʱ��ʱ�ط�������һ���������߳��˳�
 * @param arg ReSendNodeָ��
 */
void *ReSendAction(void *arg);

/**
 * ��ѯ�ط��б��ҵ��ط������Ľڵ㣬��Щ�ڵ�������߳��Ѿ��˳����ͷŽڵ㼴��
 */
void *pollOverTimeReSendNode();




#endif
