
//udp套接字读入输出函数实现
#include "private_sockio.h"


//加锁函数，不用担心死锁问题
int my_mutex_lock(pthread_mutex_t *mutex)
{
	int i = 5;
	while(i--)
	{
		if(!pthread_mutex_trylock(mutex))
			return 0;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 200000;  //200ms
		select(0,NULL,NULL,NULL,&tv);
	}
	return -1;
}

//socket发送函数
uint16 my_sendto(int fd,const void* msg,int len,unsigned int flags,const struct sockaddr* to,socklen_t tolen)
{
    //开始写
	//debug("fd = %d, addr = %d, port = %d\n",fd,((struct sockaddr_in*)to)->sin_addr.s_addr,((struct sockaddr_in*)to)->sin_port);
	uint16 sendlen = sendto(fd,msg,len,flags,to,tolen);

	return sendlen;
}

//socket接收函数
uint16 my_recvfrom(int fd,void *buf,unsigned int flags,struct sockaddr *from,socklen_t *fromlen)
{
	int len = recvfrom(fd,buf,MAXDATALENGTH,flags,from,fromlen);
	if(len == -1)
	{
		printf("error info: %s\n",strerror(errno));
		return 0u;
	}
	return (uint16)len;
}
