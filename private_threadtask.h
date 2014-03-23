
#ifndef THREADTASK_H_
#define THREADTASK_H_

/** 线程任务函数声明 **/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "private_sockio.h"
#include "private_sendlist.h"
#include "private_resend.h"
#include "private_dbio.h"
#include "private_debug.h"
#include "private_protocol.h"
#include "private_realtime.h"

typedef struct report_info{
	int sockfd;   //套接字
	char *msg;	 //汇报消息
	uint16 len;  //消息长度
	struct sockaddr_in agentaddr;  //代理服务器地址
}ReportInfo;

//接收命令的数据结构
struct recv_command{
	int sockfd;
	struct sockaddr_in recv_addr;
	char buf[MAXDATALENGTH];
	uint16 buflen;
};
typedef struct recv_command RecvCommand;


//代理服务器ip地址
extern char *agent_ip;
//代理服务器端口
extern uint16 agent_port;
//自己账户
extern char * self_account;


/**
 * 协议解析入口函数
 * @param arg 强制转换为RecvCommand类型进行操作
 */
void *parseCommand(void *arg);

/**
 * 初始化报告结构信息
 * @param sockfd 套接字文件描述符
 * @return ReportInfo 返回生成的信息结构
 */
ReportInfo* initReportInfo(int sockfd);

/**
 * 线程函数，机房服务器定时report自己的ip和端口信息
 * @param arg 强制转换为ReportInfo结构进行操作
 */
void *pollReportInfo(void *arg);



#endif
