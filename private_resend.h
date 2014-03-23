
#ifndef RESEND_H_
#define RESEND_H_

/**定时发送信息结构与函数的定义文件**/

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


#define PERIOD 200000				//发送时间间隔,单位微妙us，也就是留给客户端回复的最长时间间隔，如果服务器在该事件间隔内未收到回复信息，则重发信息

#define MAX_TIME  5              //重发的最大次数

enum ReSendType{
	RESEND_NONE,
	RESEND_ALARM,		//	报警类型
	RESEND_HOLE,	    //打洞类型
	RESEND_LEVEL_NODEON,		//液位节点出现类型
	RESEND_LEVEL_NODEOFF,		//液位节点消失类型
	RESEND_TEMP_NODEON,		//液位节点出现类型
	RESEND_TEMP_NODEOFF		//液位节点消失类型
};

//这个列表的主键是ip，port，type，drum
typedef struct resend_node{
	int sockfd;
	struct sockaddr_in sendaddr;
	char *buf;
	uint16 buflen;
	uint8 timer;
	enum ReSendType type;					//重发类型，服务器有四种重发类型：报警、打洞、节点出现和消失
	uint8 drum;
	pthread_t tid;
	pthread_mutex_t lock;
	struct resend_node *next;
}ReSendNode;

//重发列表
extern ReSendNode *ReSendList;

/**
 * 加入超时重发列表
 * @param fd 套接字描述符
 * @param addr 要发送的网络地址
 * @param buf 要发送的协议内容
 * @param len 要发送的协议长度
 * @param type 要发送的消息类型RESEND_ALARM，RESEND_HOLE，RESEND_NODEON，RESEND_NODEOFF共四种
 * @param drum 要发送的消息对应的桶号
 * @return 0 加入成功，其他失败
 */
int InTimelyReSendQueue(int fd,struct sockaddr_in *addr,char *buf,uint16 len,enum ReSendType type,uint8 drum);

/**
 * 节点退出超时重发列表，不再重发信息,当type为RESEND_HOLE时，drum为0，其他时候drum不为0
 * @param addr 消息来源的网络地址
 * @param type 消息类型RESEND_ALARM，RESEND_HOLE，RESEND_NODEON，RESEND_NODEOFF共四种
 * @param drum 桶号，当type为RESEND_HOLE时，drum为0，其他时刻不可能为0
 * @return 0 表示退出成功，其他失败
 */
int QuitReSendQueue(struct sockaddr_in *addr,enum ReSendType type,uint8 drum);

/**
 * 完成超时重发过程的线程函数，每个线程对应一个ReSendNode，定时超时重发，超过一定次数后，线程退出
 * @param arg ReSendNode指针
 */
void *ReSendAction(void *arg);

/**
 * 轮询重发列表，找到重发结束的节点，这些节点关联的线程已经退出，释放节点即可
 */
void *pollOverTimeReSendNode();




#endif
