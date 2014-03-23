
#ifndef PROTOCOL_H_
#define PROTOCOL_H_


//协议处理与生成函数声明
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "private_debug.h"
#include "private_dbio.h"


//协议各字段的长度（字节数）
#define size_SerialNum 8   //序列号长度
#define size_Length 2      //length长度
#define size_Type 1        //类型长度
#define size_Drum 1        //油桶号长度
#define size_XTime 4       //各个时间的长度
#define size_CheckSum 1    //校验码长度
#define size_Split 1       //Data各项的分割位(此处为'\0')长度

enum RealLabel{				//实时信息请求与传输标签
	REAL_NONE,				//空
	REAL_LTINFO_TRANS,		//实时液位和温度信息发送
	REAL_VIDEO_TRANS,		//实时视频信息发送
	REAL_ALARM_FISRT,		//实时第一级报警
	REAL_ALARM_SECOND,		//实时第二级报警
	REAL_ALARM_THIRD,		//实时第三级报警
	REAL_LEVEL_NODE_ON,		//液位节点活了
	REAL_LEVEL_NODE_OFF,	//液位节点死去
	REAL_TEMP_NODE_ON,		//温度节点活了
	REAL_TEMP_NODE_OFF		//温度节点死去
};

//返回的实时信息结构
typedef struct real_info{
	uint8 drum;
	uint16 level;
	float temp;
	struct real_info *next;
}RealInfo;



//分割位
extern const char SPLIT;


//命令类型不能定义为枚举常量，枚举常量为4字节有符号，和无符号数比对会出问题

//报告ip命令类型
extern const uint8 CMD_REPORT;
//登陆请求命令
extern const uint8 CMD_REQLOGIN;
//回复登陆请求命令
extern const uint8 CMD_RPYLOGIN;
//液位查询请求命令
extern const uint8 CMD_REQLEVEL;
//液位查询回复命令
extern const uint8 CMD_RPYLEVEL;
//温度查询请求命令
extern const uint8 CMD_REQTEMP;
//温度查询回复命令
extern const uint8 CMD_RPYTEMP;
//开阀记录查询请求命令
extern const uint8 CMD_REQLOCK;
//开发记录查询回复命令
extern const uint8 CMD_RPYLOCK;
//视频请求命令
extern const uint8 CMD_REQVIDEO;
//视频回复命令
extern const uint8 CMD_RPYVIDEO;
//取消视频实时发送命令
extern const uint8 CMD_CNLVIDEO;
//打洞请求命令
extern const uint8 CMD_REQHOLE;
//打洞回复命令
extern const uint8 CMD_RPYHOLE;
//报警信息命令
extern const uint8 CMD_ALARM;
//液位仪出现或退出命令
extern const uint8 CMD_LEVELINOUT;
//温度传感器出现或退出命令
extern const uint8 CMD_TEMPINOUT;
//液位和温度实时请求协议
extern const uint8 CMD_REQREALLT;
//液位和温度实时信息回复协议
extern const uint8 CMD_RPLREALLT;
//取消实时液位和温度传输协议
extern const uint8 CMD_CNLREALLT;
//液位温度上传协议
extern const uint8 CMD_LTREPORT;
//开锁信息上传协议
extern const uint8 CMD_OLREPORT;



/**
 * 请求错误协议，所有不能由服务器正常完成的请求协议，其回复都是本协议
 * @param serial 序列号
 * @param type 请求类型，回复类型为请求类型加一
 * @param username 用户名
 * @param len 协议长度
 * @return 协议串
 */
char *getRpyErrorReq(char *serial,uint8 type,char *username,uint16 *len);

/**
 * 获取历史液位查询结果回复协议，返回结果需要显式释放
 * @param username 协议用户名
 * @param serial 协议头序列串
 * @param info 液位信息序列
 * @param len 函数改写的协议长度指针
 * @return 协议序列
 */
char *getRpyLevelHistory(char *username ,char *serial,LTInfo *info,uint16 *len);

/**
 * 获取历史温度查询结果回复协议，返回结果需要显式释放
 * @param username 协议用户名
 * @param serial 协议头序列串
 * @param info 液位信息序列
 * @param len 函数改写的协议长度指针
 * @return 协议序列
 */
char *getRpyTempHistory(char *username ,char *serial,LTInfo *info,uint16 *len);

/**
 * 获取开锁记录查询协议
 * @param username 协议用户名
 * @param serial 协议头序列串
 * @param info 开锁信息序列
 * @param len 函数改写的协议长度指针
 * @return 协议序列
 */
char *getRpyLockHistory(char *username ,char *serial,OLInfo *info,uint16 *len);

/**
 * 获取打洞请求传输协议
 * @param len 函数改写的协议长度指针
 * @return 协议序列 
 */
char * getHole(uint16 *len);

/**
 * 获取打洞停止协议
 * @param serial 协议头序列串
 * @param len 函数改写的协议长度指针
 * @return 协议序列
 */
char * getStopHole(char *serial,uint16 *len);

/**
 * 获取实时信息回复协议
 * @param username 协议用户名
 * @param info 实时液位温度信息序列
 * @param len 函数改写的协议长度指针
 * @return 协议序列 
 */
char *getRealLTInfo(RealInfo *info,char *username,uint16 *len);

/**
 * 获取报警信息传输协议
 * @param username 协议用户名
 * @param drum 桶号
 * @param time 时间序列头
 * @param type 报警类型
 * @param leveldiff液位差值
 * @param len 函数改写的协议长度指针
 * @return 协议序列
 */
char *getAlarmInfo(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint32 leveldiff,uint16 *len);

/**
 * 获取节点死活信息传输协议
 * @param username 协议用户名
 * @param drum 桶号
 * @param time 时间序列头
 * @param type 节点状态类型
 * @param len 函数改写的协议长度指针
 * @return 协议序列
 */
char *getNodeOnOffReport(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint16 *len);

/**
 * 获取校验位
 * @param data 要校验的数据
 * @param len校验数据的长度，校验位放在最后一字节上，即len-1上
 * @return 校验后的数据序列
 */
char* getCheckSum(char *data,uint16 len);

/**
 * 检查校验码
 * @param data 要校验的数据
 * @param len校验数据的长度，默认校验位放在最后一字节
 * @return 0表示校验成功，其他失败
 */
uint8 checkSum(char *data,uint16 len);








#endif
