
//Э��Ľ��������ɺ���
#include "private_protocol.h"

//�ָ�λ
const char SPLIT = '\0';

//����ip��������
const uint8 CMD_REPORT = 0x00;
//��½��������
const uint8 CMD_REQLOGIN = 0x01;
//�ظ���½��������
const uint8 CMD_RPYLOGIN = 0x02;
//Һλ��ѯ��������
const uint8 CMD_REQLEVEL = 0x03;
//Һλ��ѯ�ظ�����
const uint8 CMD_RPYLEVEL = 0x04;
//�¶Ȳ�ѯ��������
const uint8 CMD_REQTEMP = 0x05;
//�¶Ȳ�ѯ�ظ�����
const uint8 CMD_RPYTEMP = 0x06;
//������¼��ѯ��������
const uint8 CMD_REQLOCK = 0x07;
//������¼��ѯ�ظ�����
const uint8 CMD_RPYLOCK = 0x08;
//��Ƶ��������
const uint8 CMD_REQVIDEO = 0x09;
//��Ƶ�ظ�����
const uint8 CMD_RPYVIDEO = 0x10;
//ȡ����Ƶʵʱ��������
const uint8 CMD_CNLVIDEO = 0xF9;
//����������
const uint8 CMD_REQHOLE = 0xFE;
//�򶴻ظ�����
const uint8 CMD_RPYHOLE = 0xFF;
//������Ϣ����
const uint8 CMD_ALARM = 0xF0;
//Һλ�ǳ��ֻ��˳�����
const uint8 CMD_LEVELINOUT = 0xF1;
//�¶ȴ��������ֻ��˳�����
const uint8 CMD_TEMPINOUT = 0xF2;
//Һλ���¶�ʵʱ����Э��
const uint8 CMD_REQREALLT = 0x0A;
//Һλ���¶�ʵʱ��Ϣ�ظ�Э��
const uint8 CMD_RPLREALLT = 0x0B;
//ȡ��ʵʱҺλ���¶ȴ���Э��
const uint8 CMD_CNLREALLT = 0xFA;
//Һλ���¶��ϴ�Э��
const uint8 CMD_LTREPORT = 0xB0;
//������Ϣ�ϴ�Э��
const uint8 CMD_OLREPORT = 0xB1;


//�������Э�飬���в����ɷ�����������ɵ�����Э�飬��ظ����Ǳ�Э��
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
	*(uint8 *)tmp = type + 1; //��1��Ϊ�ظ�����
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


//��ȡ��ʷҺλ��ѯ����ظ�Э�飬���ؽ����Ҫ��ʽ�ͷ�
char *getRpyLevelHistory(char *username ,char *serial,LTInfo *info,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);  //�����븳ֵ
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(char *)tmp = CMD_RPYLEVEL;	//type��ֵ
	tmp += size_Type;
	buflen += size_Type;
	//��ȡdata����
	memcpy(tmp,username,strlen(username));  //��ֵusername
	int size = strlen(username);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;   //�ָ�λ
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
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //��length��ֵ
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}


//��ȡ��ʷ�¶Ȳ�ѯ����ظ�Э�飬���ؽ����Ҫ��ʽ�ͷ�
char *getRpyTempHistory(char *username ,char *serial,LTInfo *info,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);  //�����븳ֵ
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(char *)tmp = CMD_RPYTEMP;	//type��ֵ
	tmp += size_Type;
	buflen += size_Type;
	//��ȡdata����
	memcpy(tmp,username,strlen(username));  //��ֵusername
	int size = strlen(username);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;   //�ָ�λ
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
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //��length��ֵ
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}

//��ȡ������¼��ѯЭ��
char *getRpyLockHistory(char *username ,char *serial,OLInfo *info,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,serial,size_SerialNum);  //�����븳ֵ
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;
	buflen += size_Length;
	*(char *)tmp = CMD_RPYLOCK;	//type��ֵ
	tmp += size_Type;
	buflen += size_Type;
	//��ȡdata����
	memcpy(tmp,username,strlen(username));  //��ֵusername
	int size = strlen(username);
	tmp += size;
	buflen += size;
	*(char *)tmp = SPLIT;   //�ָ�λ
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
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //��length��ֵ
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}

//��ȡ��������Э��
char * getHole(uint16 *len)
{
	int buflen = size_SerialNum + size_Length + size_Type + size_Split + size_CheckSum;
	char *buf = (char *)malloc(sizeof(char) * buflen);
	memset(buf,0,sizeof(char) * buflen);
	char *tmp = buf;
	tmp += size_SerialNum;    //��Э���serialȫΪ0
	*(uint16 *)tmp = buflen - size_SerialNum;
	tmp += size_Length;
	*(char *)tmp = CMD_REQHOLE;
	tmp += size_Type;
	*(char *)tmp = SPLIT;
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}

//��ȡ��ֹͣЭ��
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
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}

//��ȡʵʱ��Ϣ�ظ�Э��
char *getRealLTInfo(RealInfo *info,char *username,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	uint64 serial = getCurrentTimeSeconds();   //��ʱ����ΪЭ��ͷ
	memcpy(tmp,(char *)&serial,sizeof(serial));
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;       //����length����
	buflen += size_Length;
	*(char *)tmp = CMD_RPLREALLT;
	tmp += size_Type;
	buflen += size_Type;
	
	//��ȡdata����
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
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //��length��ֵ
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}

//��ȡ������Ϣ����Э��
char *getAlarmInfo(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint32 leveldiff,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,time,sizeof(uint64));  //��ʱ����ΪЭ��ͷ
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;       //����length����
	buflen += size_Length;
	*(char *)tmp = CMD_ALARM;
	tmp += size_Type;
	buflen += size_Type;
	
	//��ȡdata����
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
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //��length��ֵ
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}

//��ȡ�ڵ�������Ϣ����Э��
char *getNodeOnOffReport(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint16 *len)
{
	char *buf = (char *)malloc(MAXDATALENGTH * sizeof(char));
	memset(buf,0,MAXDATALENGTH * sizeof(char));
	char *tmp = buf;
	uint16 buflen = 0;
	memcpy(tmp,time,sizeof(uint64));  //��ʱ����ΪЭ��ͷ
	tmp += size_SerialNum;
	buflen += size_SerialNum;
	tmp += size_Length;       //����length����
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
	
	//��ȡdata����
	int size = 0;
	sprintf(tmp,"%s%c%1u%c%llu%c%1u%c",username,SPLIT,drum,SPLIT,*time,SPLIT,inout,SPLIT);
	for(int i = 0;i < 4;i++)
	{
		size = strlen(tmp) + 1;
		buflen += size;
		tmp += size;
	}
	
	buflen += size_CheckSum;
	*(uint16 *)(buf + size_SerialNum) = buflen - size_SerialNum;    //��length��ֵ
	getCheckSum(buf,buflen);  //��ȡУ��λ
	
	*len = buflen;
	return buf;
}


//��ȡУ��λ
char* getCheckSum(char *data,uint16 len)
{
	data[len -1] = 0;
	for(int i = 0; i < len - 1; i++)
		data[len - 1] ^= data[i];

	return data;	
}

//���У����
uint8 checkSum(char *data,uint16 len)
{
	char checkByte = data[0];
	for(int i = 1; i < len; i++)
		checkByte ^= data[i];

	return checkByte;
}







