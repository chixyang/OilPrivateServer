
#ifndef REALTIME_H_
#define REALTIME_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "private_dbio.h"
#include "private_debug.h"
#include "private_sendlist.h"
#include "private_resend.h"
#include "private_protocol.h"

//extern enum RealLabel;

//extern typedef struct real_info RealInfo;

//实时信息发送列表
typedef struct real_send{
	char username[20];	                           //用户名
	uint8 drum;										//桶号
	int fd;										   //套接字文件描述符
	struct sockaddr_in sendaddr;				   //发送地址
	enum RealLabel type; 										//发送类型REAL_LTINFO_TRANS或者REAL_VIDEO_TRANS（视频）
	uint8 timer;									//发送次数，一般设置为FF，一分钟发一次，所以可以发送255分钟，所以实时发送并不是永久发送，还是有时间限制的
	pthread_t tid;
	pthread_mutex_t lock;							//节点锁，主要保护timer
	struct real_send *next;
}RealSendNode;

extern RealSendNode *RealSendList;        //实时发送列表

#define REAL_SEND_TIMER 0xFF    //设置实时发送的次数

#define REAL_SEND_PERIOD 60 	//实际值应该为60,设置实时发送两次之间的时间间隔，单位秒


//第一级液位节点
struct level_temp_first_node{
	uint64 time;		//当前时间
	uint8 drum;			//桶号
	union{
		uint16 level;		//液位高度
		float temp;			//温度
	}lt;
	pthread_mutex_t lock;  //节点锁
	uint8 active;			//确定节点是否活跃，1为活跃，0为down掉
	struct level_temp_first_node *next;
};
typedef struct level_temp_first_node LevelFirstNode;   //第一层液位节点
typedef struct level_temp_first_node TempFirstNode;   //第一层温度节点

#define LevelFirstDiff 5        //第一级液位最高允许偏差，超过此偏差就报警，单位毫米，采用计数器算，每一个值比较

#define LevelSecondDiff 20 		//第二级液位最高允许偏差，超过此偏差报警，单位毫米，采用计数器算（防止时间带来的不准），每60个第一级值的和比较
#define Max_Second_Num 60

#define LevelThirdDiff 50		//第三级液位最高允许偏差，超过此偏差报警，单位毫米，采用计数器算（防止时间带来的不准），每12个第二级值的和比较
#define Max_Third_Num 12

//第二三层液位节点
struct level_temp_scd_thd_node{
	uint64 time;			//当前时间
	uint8 drum;				//桶号
	union{
		uint32 sum_level;		//液位高度和
		float sum_temp;			//温度和
	}lt;
	uint16 num;				//收到记录的数量
	pthread_mutex_t lock;  //节点锁
	//uint8 active;			//这里就不加这个标记位，所有的实时信息访问都要先经过第一级节点判断节点是否active，然后在进入相应的节点
	struct level_temp_scd_thd_node *next;
};
typedef struct level_temp_scd_thd_node LevelSecondNode;   //第二层液位节点
typedef struct level_temp_scd_thd_node LevelThirdNode;    //第三层液位节点
typedef struct level_temp_scd_thd_node TempSecondNode;   //第二层温度节点
typedef struct level_temp_scd_thd_node TempThirdNode;    //第三层温度节点

//油桶数量
extern const uint8 NumofDrums;

//第一级节点到第二级节点的时间差
#define FSTTOSEC 3600          //本来应为3600,60min*60s，因为time函数的精度是秒
//第二级节点到第三级节点的时间差
#define SECTOTHD 43200         //本来应为43200,12h*60min*60s

//第一级液位节点列表
extern LevelFirstNode *LevelFirstList;
//第二级液位节点列表
extern LevelSecondNode *LevelSecondList;
//第三级液位节点列表
extern LevelThirdNode *LevelThirdList;

//第一级温度节点列表
extern TempFirstNode *TempFirstList;
//第二级液位节点列表
extern TempSecondNode *TempSecondList;
//第三级液位节点列表
extern TempThirdNode *TempThirdList;

#define PollDiedTime 120    //轮询节点状态时间2min * 60s = 120s
#define ActiveDiff 360		//6min * 60s = 360s,表示如果现在距离上一次活跃的时间大于等于6分钟，即六次没收到底层传输的数据了，则证明节点设备坏了


/**
 * 初始化液位记录和与计数器，初始化实时信息列表
 */
void initRealLTList();

/**
 * 添加来自于底层设备的新数据信息,数据取地址
 * @param type 信息类型，只能为L_FIRST和T_FIRST两者之一
 * @param info 信息指针，uint16或者float类型的指针
 * @param drum 来源的桶号
 * @return 0 添加成功，其他失败
 */
int addNewLTInfo(enum LTLabel type,void *info,uint8 drum);

/**
 * 查询实时数据信息,查询结果仅为first 第一级内容，其他级内容不为实时内容
 * @param drum 桶号，当drum为0时，表示全部查询
 * @return RealInfo列表，需要显式释放，NULL未查询到结果
 */
RealInfo *getCurrentLTInfo(uint8 drum);

/**
 * 释放RealInfo列表
 * @param ri RealInfo列表指针
 */
void freeRealInfoList(RealInfo *ri);

/**
 * 线程函数，轮询LevelFirstList和TempFirstList列表，查看是否有节点超时未收到数据，超时则认为died
 */
void *pollDiedNode();

/**
 * 请求实时信息定时发送，加入实时发送列表
 * @param username 请求用户
 * @param drum 所请求的桶号，0表示请求全部桶的信息
 * @param fd 当前套接字fd
 * @param addr 用户的地址
 * @param 请求的信息类型：	REAL_LTINFO_TRANS和REAL_VIDEO_TRANS两种类型
 * @return 0 加入成功，其他失败
 */
int InRealSendList(char *username,uint8 drum,int fd,struct sockaddr_in *addr, enum RealLabel type);

/**
 * 退出实时信息定时发送列表，该退出过程必须要客户端发信息才会完成的
 * @param username 请求用户
 * @param drum 桶号
 * @param type 之前对应的发送类型：REAL_LTINFO_TRANS和REAL_VIDEO_TRANS两种类型
 * @return 0 退出成功，其他失败
 */
int QuitRealSendList(char *username,uint8 drum,enum RealLabel type);

/**
 * 线程函数，实时信息定时发送函数
 * @param agr 强制转换为RealSendNode结构进行操作
 */
void *RTSendAction(void *arg);

/**
 * 广播发送节点加入或者退出消息
 * @param time 时间标签
 * @param type为RealLabel枚举常量，取值范围为NODE_ON，NODE_OFF，这些是重要消息，只要是列表上的节点都要发送通知，并且要确保收到，所以采用定时发送机制
 * @param drum 桶号
 * @return 0表示发送成功，其他失败
 */
int sendNodeOnOffBroadInfo(uint64 *time,enum RealLabel type,uint8 drum);

/**
 * 广播发送节点液位报警信息
 * @param time 时间标签
 * @param type ,RealLabel枚举常量,取值为alarm系列
 * @param drum 桶号
 * @param leveldiff 液位差值
 * @param 0表示发送成功，其他失败
 */
int sendAlarmBroadInfo(uint64 *time,enum RealLabel type,uint8 drum,uint32 leveldiff);

/**
 * 轮询超时的实时节点，找到并删除
 */
void *pollOverTimeRTNode();





#endif
