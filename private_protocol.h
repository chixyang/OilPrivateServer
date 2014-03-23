
#ifndef PROTOCOL_H_
#define PROTOCOL_H_


//Э�鴦�������ɺ�������
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


//Э����ֶεĳ��ȣ��ֽ�����
#define size_SerialNum 8   //���кų���
#define size_Length 2      //length����
#define size_Type 1        //���ͳ���
#define size_Drum 1        //��Ͱ�ų���
#define size_XTime 4       //����ʱ��ĳ���
#define size_CheckSum 1    //У���볤��
#define size_Split 1       //Data����ķָ�λ(�˴�Ϊ'\0')����

enum RealLabel{				//ʵʱ��Ϣ�����봫���ǩ
	REAL_NONE,				//��
	REAL_LTINFO_TRANS,		//ʵʱҺλ���¶���Ϣ����
	REAL_VIDEO_TRANS,		//ʵʱ��Ƶ��Ϣ����
	REAL_ALARM_FISRT,		//ʵʱ��һ������
	REAL_ALARM_SECOND,		//ʵʱ�ڶ�������
	REAL_ALARM_THIRD,		//ʵʱ����������
	REAL_LEVEL_NODE_ON,		//Һλ�ڵ����
	REAL_LEVEL_NODE_OFF,	//Һλ�ڵ���ȥ
	REAL_TEMP_NODE_ON,		//�¶Ƚڵ����
	REAL_TEMP_NODE_OFF		//�¶Ƚڵ���ȥ
};

//���ص�ʵʱ��Ϣ�ṹ
typedef struct real_info{
	uint8 drum;
	uint16 level;
	float temp;
	struct real_info *next;
}RealInfo;



//�ָ�λ
extern const char SPLIT;


//�������Ͳ��ܶ���Ϊö�ٳ�����ö�ٳ���Ϊ4�ֽ��з��ţ����޷������ȶԻ������

//����ip��������
extern const uint8 CMD_REPORT;
//��½��������
extern const uint8 CMD_REQLOGIN;
//�ظ���½��������
extern const uint8 CMD_RPYLOGIN;
//Һλ��ѯ��������
extern const uint8 CMD_REQLEVEL;
//Һλ��ѯ�ظ�����
extern const uint8 CMD_RPYLEVEL;
//�¶Ȳ�ѯ��������
extern const uint8 CMD_REQTEMP;
//�¶Ȳ�ѯ�ظ�����
extern const uint8 CMD_RPYTEMP;
//������¼��ѯ��������
extern const uint8 CMD_REQLOCK;
//������¼��ѯ�ظ�����
extern const uint8 CMD_RPYLOCK;
//��Ƶ��������
extern const uint8 CMD_REQVIDEO;
//��Ƶ�ظ�����
extern const uint8 CMD_RPYVIDEO;
//ȡ����Ƶʵʱ��������
extern const uint8 CMD_CNLVIDEO;
//����������
extern const uint8 CMD_REQHOLE;
//�򶴻ظ�����
extern const uint8 CMD_RPYHOLE;
//������Ϣ����
extern const uint8 CMD_ALARM;
//Һλ�ǳ��ֻ��˳�����
extern const uint8 CMD_LEVELINOUT;
//�¶ȴ��������ֻ��˳�����
extern const uint8 CMD_TEMPINOUT;
//Һλ���¶�ʵʱ����Э��
extern const uint8 CMD_REQREALLT;
//Һλ���¶�ʵʱ��Ϣ�ظ�Э��
extern const uint8 CMD_RPLREALLT;
//ȡ��ʵʱҺλ���¶ȴ���Э��
extern const uint8 CMD_CNLREALLT;
//Һλ�¶��ϴ�Э��
extern const uint8 CMD_LTREPORT;
//������Ϣ�ϴ�Э��
extern const uint8 CMD_OLREPORT;



/**
 * �������Э�飬���в����ɷ�����������ɵ�����Э�飬��ظ����Ǳ�Э��
 * @param serial ���к�
 * @param type �������ͣ��ظ�����Ϊ�������ͼ�һ
 * @param username �û���
 * @param len Э�鳤��
 * @return Э�鴮
 */
char *getRpyErrorReq(char *serial,uint8 type,char *username,uint16 *len);

/**
 * ��ȡ��ʷҺλ��ѯ����ظ�Э�飬���ؽ����Ҫ��ʽ�ͷ�
 * @param username Э���û���
 * @param serial Э��ͷ���д�
 * @param info Һλ��Ϣ����
 * @param len ������д��Э�鳤��ָ��
 * @return Э������
 */
char *getRpyLevelHistory(char *username ,char *serial,LTInfo *info,uint16 *len);

/**
 * ��ȡ��ʷ�¶Ȳ�ѯ����ظ�Э�飬���ؽ����Ҫ��ʽ�ͷ�
 * @param username Э���û���
 * @param serial Э��ͷ���д�
 * @param info Һλ��Ϣ����
 * @param len ������д��Э�鳤��ָ��
 * @return Э������
 */
char *getRpyTempHistory(char *username ,char *serial,LTInfo *info,uint16 *len);

/**
 * ��ȡ������¼��ѯЭ��
 * @param username Э���û���
 * @param serial Э��ͷ���д�
 * @param info ������Ϣ����
 * @param len ������д��Э�鳤��ָ��
 * @return Э������
 */
char *getRpyLockHistory(char *username ,char *serial,OLInfo *info,uint16 *len);

/**
 * ��ȡ��������Э��
 * @param len ������д��Э�鳤��ָ��
 * @return Э������ 
 */
char * getHole(uint16 *len);

/**
 * ��ȡ��ֹͣЭ��
 * @param serial Э��ͷ���д�
 * @param len ������д��Э�鳤��ָ��
 * @return Э������
 */
char * getStopHole(char *serial,uint16 *len);

/**
 * ��ȡʵʱ��Ϣ�ظ�Э��
 * @param username Э���û���
 * @param info ʵʱҺλ�¶���Ϣ����
 * @param len ������д��Э�鳤��ָ��
 * @return Э������ 
 */
char *getRealLTInfo(RealInfo *info,char *username,uint16 *len);

/**
 * ��ȡ������Ϣ����Э��
 * @param username Э���û���
 * @param drum Ͱ��
 * @param time ʱ������ͷ
 * @param type ��������
 * @param leveldiffҺλ��ֵ
 * @param len ������д��Э�鳤��ָ��
 * @return Э������
 */
char *getAlarmInfo(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint32 leveldiff,uint16 *len);

/**
 * ��ȡ�ڵ�������Ϣ����Э��
 * @param username Э���û���
 * @param drum Ͱ��
 * @param time ʱ������ͷ
 * @param type �ڵ�״̬����
 * @param len ������д��Э�鳤��ָ��
 * @return Э������
 */
char *getNodeOnOffReport(char *username,uint8 drum,uint64 *time,enum RealLabel type,uint16 *len);

/**
 * ��ȡУ��λ
 * @param data ҪУ�������
 * @param lenУ�����ݵĳ��ȣ�У��λ�������һ�ֽ��ϣ���len-1��
 * @return У������������
 */
char* getCheckSum(char *data,uint16 len);

/**
 * ���У����
 * @param data ҪУ�������
 * @param lenУ�����ݵĳ��ȣ�Ĭ��У��λ�������һ�ֽ�
 * @return 0��ʾУ��ɹ�������ʧ��
 */
uint8 checkSum(char *data,uint16 len);








#endif
