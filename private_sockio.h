
#ifndef SOCKIO_H_
#define SOCKIO_H_

//udp操作函数声明

#include "private_debug.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


/**
  * 加锁函数，不用担心死锁问题
  * @param mutex 互斥量
 */
int my_mutex_lock(pthread_mutex_t *mutex);

/**
 * socket发送函数,参数与send_to相同
 */
uint16 my_sendto(int fd,const void* msg,int len,unsigned int flags,const struct sockaddr* to,socklen_t tolen);

/**
 * socket接收函数，参数与recv_from相同
 */
uint16 my_recvfrom(int fd,void *buf,unsigned int flags,struct sockaddr *from,socklen_t *fromlen);







#endif
