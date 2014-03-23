
//协议的解析与生成函数
#include "private_protocol.h"

//分割位
const char SPLIT = '\0';

//报告ip命令类型
const uint8 CMD_REPORT = 0x00;
//登陆请求命令
const uint8 CMD_REQLOGIN = 0x01;
//回复登陆请求命令
const uint8 CMD_RPYLOGIN = 0x02;
//液位查询请求命令
const uint8 CMD_REQLEVEL = 0x03;
//液位查询回复命令
const uint8 CMD_RPYLEVEL = 0x04;
//温度查询请求命令
const uint8 CMD_REQTEMP = 0x05;
//温度查询回复命令
const uint8 CMD_RPYTEMP = 0x06;
//开阀记录查询请求命令
const uint8 CMD_REQLOCK = 0x07;
//开发记录查询回复命令
const uint8 CMD_RPYLOCK = 0x08;
//视频请求命令
const uint8 CMD_REQVIDEO = 0x09;
//视频回复命令
const uint8 CMD_RPYVIDEO = 0x10;
//取消视频实时发送命令
const uint8 CMD_CNLVIDEO = 0xF9;
//打洞请求命令
const uint8 CMD_REQHOLE = 0xFE;
//打洞回复命令
const uint8 CMD_RPYHOLE = 0xFF;
//报警信息命令
const uint8 CMD_ALARM = 0xF0;
//液位仪出现或退出命令
const uint8 CMD_LEVELINOUT = 0xF1;
//温度传感器出现或退出命令
const uint8 CMD_TEMPINOUT = 0xF2;
//液位和温度实时请求协议
const uint8 CMD_REQREALLT = 0x0A;
//液位和温度实时信息回复协议
const uint8 CMD_RPLREALLT = 0x0B;
//取消实时液位和温度传输协议
const uint8 CMD_CNLREALLT = 0xFA;
//液位和温度上传协议
const uint8 CMD_LTREPORT = 0xB0;
//开锁信息上传协议
const uint8 CMD_OLREPORT = 0xB1;


//请求错误协议，所有不能由服务器正常完成的请求协议，其回复都是本协议
char *getRpyErrorReq(char *serial,uint8 type,char *username,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(uint8 *)tmp = type + 1; //加1即为回复命令
	tmp += size_Type;
	buflen += size_Type;
	memcpy(tmp,username,strlen(username));
	buflen += strlen(username) + size_Split;
	buflen += size_Split;
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;
	getCheckSum(buf,buflen);

	*len = buflen;
	return buf;
}


//获取历史液位查询结果回复协议，返回结果需要显式释放
char *getRpyLevelHistory(char *username ,char *serial,LTInfo *info,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);  //序列码赋值
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(char *)tmp = CMD_RPYLEVEL;	//type赋值
	tmp += size_Type;
	buflen += size_Type;
	//获取data部分
	memcpy(tmp,username,strlen(username));  //赋值username
	int size = strlen(username);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;   //分割位
	tmp++;
	buflen++;
	while(info != NULL)
	{
		sprintf(tmp,"%1u%c%llu%c%2u%c",info->drum,SPLIT,info->time,SPLIT,info->lt.level,SPLIT);
		for(int i = 0;i < 3;i++)
		{
			size = strlen(tmp) + 1;
			buflen += size;
			tmp += size;
		}
		info = info->next;
	}
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //给length赋值
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}


//获取历史温度查询结果回复协议，返回结果需要显式释放
char *getRpyTempHistory(char *username ,char *serial,LTInfo *info,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);  //序列码赋值
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(char *)tmp = CMD_RPYTEMP;	//type赋值
	tmp += size_Type;
	buflen += size_Type;
	//获取data部分
	memcpy(tmp,username,strlen(username));  //赋值username
	int size = strlen(username);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;   //分割位
	tmp++;
	buflen++;
	while(info != NULL)
	{
		sprintf(tmp,"%1u%c%llu%c%.1f%c",info->drum,SPLIT,info->time,SPLIT,info->lt.temp,SPLIT);
		for(int i = 0;i < 3;i++)
		{
			size = strlen(tmp) + 1;
			buflen += size;
			tmp += size;
		}
		info = info->next;
	}
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //给length赋值
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}

//获取开锁记录查询协议
char *getRpyLockHistory(char *username ,char *serial,OLInfo *info,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);  //序列码赋值
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(char *)tmp = CMD_RPYLOCK;	//type赋值
	tmp += size_Type;
	buflen += size_Type;
	//获取data部分
	memcpy(tmp,username,strlen(username));  //赋值username
	int size = strlen(username);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;   //分割位
	tmp++;
	buflen++;
	while(info != NULL)
	{
		sprintf(tmp,"%s%c%1u%c%llu%c%llu%c",info->username,SPLIT,info->lock,SPLIT,info->open_time,SPLIT,info->close_time,SPLIT);
		for(int i = 0;i < 4;i++)
		{
			size = strlen(tmp) + 1;
			buflen += size;
			tmp += size;
		}
		info = info->next;
	}
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //给length赋值
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}

//获取打洞请求传输协议
char * getHole(uint16 *len)
{
	int buflen = size_SerialNum + size_Length + size_Type + size_Split + size_CheckSum;
	char *buf = (char *)malloc(sizeof(char) * buflen);
	memset(buf,0,sizeof(char) * buflen);
	char *tmp = buf;
	tmp += size_SerialNum;    //打洞协议的serial全为0
	*(uint16 *)tmp = buflen - size_SerialNum;
	tmp += size_Length;
	*(char *)tmp = CMD_REQHOLE;
	tmp += size_Type;
	*(char *)tmp = SPLIT;
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}

//获取打洞停止协议
char * getStopHole(char *serial,uint16 *len)
{
	int buflen = size_SerialNum + size_Length + size_Type + size_Split + size_CheckSum;
	char *buf = (char *)malloc(sizeof(char) * buflen);
	memset(buf,0,sizeof(char) * buflen);
	char *tmp = buf;
	memcpy(buf,serial,size_SerialNum);
	tmp += size_SerialNum;
	*(uint16 *)tmp = buflen - size_SerialNum;
	tmp += size_Length;
	*(char *)tmp = CMD_RPYHOLE;
	tmp += size_Type;
	*(char *)tmp = SPLIT;
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}

//获取实时信息回复协议
char *getRealLTInfo(RealInfo *info,char *username,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	uint64 serial = getCurrentTimeSeconds();   //以时间作为协议头
	memcpy(tmp,(char *)&serial,sizeof(serial));
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;       //加上length长度
	buflen += size_Length;
	*(char *)tmp = CMD_RPLREALLT;
	tmp += size_Type;
	buflen += size_Type;
	
	//获取data数据
	int size = 0;
	size = strlen(username);
	memcpy(tmp,username,size);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;
	tmp++;
	buflen++;
	
	while(info != NULL)
	{
		sprintf(tmp,"%1u%c%2u%c%.1f%c",info->drum,SPLIT,info->level,SPLIT,info->temp,SPLIT);
		for(int i = 0;i < 3;i++)
		{
			size = strlen(tmp) + 1;
			buflen += size;
			tmp += size;
		}
		info = info->next;
	}
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //给length赋值
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}

//获取报警信息传输协议
char *getAlarmInfo(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint32 leveldiff,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,time,sizeof(uint64));  //以时间作为协议头
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;       //加上length长度
	buflen += size_Length;
	*(char *)tmp = CMD_ALARM;
	tmp += size_Type;
	buflen += size_Type;
	
	//获取data数据
	uint8 period = 0;
	if(type == REAL_ALARM_FISRT)
		period = 1;
	else if(type == REAL_ALARM_SECOND)
		period = 2;
	else if(type == REAL_ALARM_THIRD)
		period = 3;
	else
	{
		free(buf);
		return NULL;
	}
	
	int size = 0;
	sprintf(tmp,"%s%c%1u%c%llu%c%1u%c%4u%c",username,SPLIT,drum,SPLIT,*time,SPLIT,period,SPLIT,leveldiff,SPLIT);
	for(int i = 0;i < 5;i++)
	{
		size = strlen(tmp) + 1;
		buflen += size;
		tmp += size;
	}
	
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //给length赋值
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}

//获取节点死活信息传输协议
char *getNodeOnOffReport(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,time,sizeof(uint64));  //以时间作为协议头
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;       //加上length长度
	buflen += size_Length;
	uint8 inout = 0;
	if(type == REAL_LEVEL_NODE_ON)
	{
		*(char *)tmp = CMD_LEVELINOUT;
		inout = 1;
	}
	else if(type == REAL_LEVEL_NODE_OFF)
	{
		*(char *)tmp = CMD_LEVELINOUT;
		inout = 0;
	}
	else if(type == REAL_TEMP_NODE_ON)
	{
		*(char *)tmp = CMD_TEMPINOUT;
		inout = 1;
	}
	else if(type == REAL_TEMP_NODE_OFF)
	{
		*(char *)tmp = CMD_TEMPINOUT;
		inout = 0;
	}
	tmp += size_Type;
	buflen += size_Type;
	
	//获取data数据
	int size = 0;
	sprintf(tmp,"%s%c%1u%c%llu%c%1u%c",username,SPLIT,drum,SPLIT,*time,SPLIT,inout,SPLIT);
	for(int i = 0;i < 4;i++)
	{
		size = strlen(tmp) + 1;
		buflen += size;
		tmp += size;
	}
	
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //给length赋值
	getCheckSum(buf,buflen);  //获取校验位
	
	*len = buflen;
	return buf;
}


//获取校验位
char* getCheckSum(char *data,uint16 len)
{
	data[len -1] = 0;
	for(int i = 0; i < len - 1; i++)
		data[len - 1] ^= data[i];

	return data;	
}

//检查校验码
uint8 checkSum(char *data,uint16 len)
{
	char checkByte = data[0];
	for(int i = 1; i < len; i++)
		checkByte ^= data[i];

	return checkByte;
}







