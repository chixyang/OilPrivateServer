
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

//ʵʱ��Ϣ�����б�
typedef struct real_send{
	char username[20];	                           //�û���
	uint8 drum;										//Ͱ��
	int fd;										   //�׽����ļ�������
	struct sockaddr_in sendaddr;				   //���͵�ַ
	enum RealLabel type; 										//��������REAL_LTINFO_TRANS����REAL_VIDEO_TRANS����Ƶ��
	uint8 timer;									//���ʹ�����һ������ΪFF��һ���ӷ�һ�Σ����Կ��Է���255���ӣ�����ʵʱ���Ͳ��������÷��ͣ�������ʱ�����Ƶ�
	pthread_t tid;
	pthread_mutex_t lock;							//�ڵ�������Ҫ����timer
	struct real_send *next;
}RealSendNode;

extern RealSendNode *RealSendList;        //ʵʱ�����б�

#define REAL_SEND_TIMER 0xFF    //����ʵʱ���͵Ĵ���

#define REAL_SEND_PERIOD 60 	//ʵ��ֵӦ��Ϊ60,����ʵʱ��������֮���ʱ��������λ��


//��һ��Һλ�ڵ�
struct level_temp_first_node{
	uint64 time;		//��ǰʱ��
	uint8 drum;			//Ͱ��
	union{
		uint16 level;		//Һλ�߶�
		float temp;			//�¶�
	}lt;
	pthread_mutex_t lock;  //�ڵ���
	uint8 active;			//ȷ���ڵ��Ƿ��Ծ��1Ϊ��Ծ��0Ϊdown��
	struct level_temp_first_node *next;
};
typedef struct level_temp_first_node LevelFirstNode;   //��һ��Һλ�ڵ�
typedef struct level_temp_first_node TempFirstNode;   //��һ���¶Ƚڵ�

#define LevelFirstDiff 5        //��һ��Һλ�������ƫ�������ƫ��ͱ�������λ���ף����ü������㣬ÿһ��ֵ�Ƚ�

#define LevelSecondDiff 20 		//�ڶ���Һλ�������ƫ�������ƫ�������λ���ף����ü������㣨��ֹʱ������Ĳ�׼����ÿ60����һ��ֵ�ĺͱȽ�
#define Max_Second_Num 60

#define LevelThirdDiff 50		//������Һλ�������ƫ�������ƫ�������λ���ף����ü������㣨��ֹʱ������Ĳ�׼����ÿ12���ڶ���ֵ�ĺͱȽ�
#define Max_Third_Num 12

//�ڶ�����Һλ�ڵ�
struct level_temp_scd_thd_node{
	uint64 time;			//��ǰʱ��
	uint8 drum;				//Ͱ��
	union{
		uint32 sum_level;		//Һλ�߶Ⱥ�
		float sum_temp;			//�¶Ⱥ�
	}lt;
	uint16 num;				//�յ���¼������
	pthread_mutex_t lock;  //�ڵ���
	//uint8 active;			//����Ͳ���������λ�����е�ʵʱ��Ϣ���ʶ�Ҫ�Ⱦ�����һ���ڵ��жϽڵ��Ƿ�active��Ȼ���ڽ�����Ӧ�Ľڵ�
	struct level_temp_scd_thd_node *next;
};
typedef struct level_temp_scd_thd_node LevelSecondNode;   //�ڶ���Һλ�ڵ�
typedef struct level_temp_scd_thd_node LevelThirdNode;    //������Һλ�ڵ�
typedef struct level_temp_scd_thd_node TempSecondNode;   //�ڶ����¶Ƚڵ�
typedef struct level_temp_scd_thd_node TempThirdNode;    //�������¶Ƚڵ�

//��Ͱ����
extern const uint8 NumofDrums;

//��һ���ڵ㵽�ڶ����ڵ��ʱ���
#define FSTTOSEC 3600          //����ӦΪ3600,60min*60s����Ϊtime�����ľ�������
//�ڶ����ڵ㵽�������ڵ��ʱ���
#define SECTOTHD 43200         //����ӦΪ43200,12h*60min*60s

//��һ��Һλ�ڵ��б�
extern LevelFirstNode *LevelFirstList;
//�ڶ���Һλ�ڵ��б�
extern LevelSecondNode *LevelSecondList;
//������Һλ�ڵ��б�
extern LevelThirdNode *LevelThirdList;

//��һ���¶Ƚڵ��б�
extern TempFirstNode *TempFirstList;
//�ڶ���Һλ�ڵ��б�
extern TempSecondNode *TempSecondList;
//������Һλ�ڵ��б�
extern TempThirdNode *TempThirdList;

#define PollDiedTime 120    //��ѯ�ڵ�״̬ʱ��2min * 60s = 120s
#define ActiveDiff 360		//6min * 60s = 360s,��ʾ������ھ�����һ�λ�Ծ��ʱ����ڵ���6���ӣ�������û�յ��ײ㴫��������ˣ���֤���ڵ��豸����


/**
 * ��ʼ��Һλ��¼�������������ʼ��ʵʱ��Ϣ�б�
 */
void initRealLTList();

/**
 * ��������ڵײ��豸����������Ϣ,����ȡ��ַ
 * @param type ��Ϣ���ͣ�ֻ��ΪL_FIRST��T_FIRST����֮һ
 * @param info ��Ϣָ�룬uint16����float���͵�ָ��
 * @param drum ��Դ��Ͱ��
 * @return 0 ��ӳɹ�������ʧ��
 */
int addNewLTInfo(enum LTLabel type,void *info,uint8 drum);

/**
 * ��ѯʵʱ������Ϣ,��ѯ�����Ϊfirst ��һ�����ݣ����������ݲ�Ϊʵʱ����
 * @param drum Ͱ�ţ���drumΪ0ʱ����ʾȫ����ѯ
 * @return RealInfo�б���Ҫ��ʽ�ͷţ�NULLδ��ѯ�����
 */
RealInfo *getCurrentLTInfo(uint8 drum);

/**
 * �ͷ�RealInfo�б�
 * @param ri RealInfo�б�ָ��
 */
void freeRealInfoList(RealInfo *ri);

/**
 * �̺߳�������ѯLevelFirstList��TempFirstList�б��鿴�Ƿ��нڵ㳬ʱδ�յ����ݣ���ʱ����Ϊdied
 */
void *pollDiedNode();

/**
 * ����ʵʱ��Ϣ��ʱ���ͣ�����ʵʱ�����б�
 * @param username �����û�
 * @param drum �������Ͱ�ţ�0��ʾ����ȫ��Ͱ����Ϣ
 * @param fd ��ǰ�׽���fd
 * @param addr �û��ĵ�ַ
 * @param �������Ϣ���ͣ�	REAL_LTINFO_TRANS��REAL_VIDEO_TRANS��������
 * @return 0 ����ɹ�������ʧ��
 */
int InRealSendList(char *username,uint8 drum,int fd,struct sockaddr_in *addr, enum RealLabel type);

/**
 * �˳�ʵʱ��Ϣ��ʱ�����б����˳����̱���Ҫ�ͻ��˷���Ϣ�Ż���ɵ�
 * @param username �����û�
 * @param drum Ͱ��
 * @param type ֮ǰ��Ӧ�ķ������ͣ�REAL_LTINFO_TRANS��REAL_VIDEO_TRANS��������
 * @return 0 �˳��ɹ�������ʧ��
 */
int QuitRealSendList(char *username,uint8 drum,enum RealLabel type);

/**
 * �̺߳�����ʵʱ��Ϣ��ʱ���ͺ���
 * @param agr ǿ��ת��ΪRealSendNode�ṹ���в���
 */
void *RTSendAction(void *arg);

/**
 * �㲥���ͽڵ��������˳���Ϣ
 * @param time ʱ���ǩ
 * @param typeΪRealLabelö�ٳ�����ȡֵ��ΧΪNODE_ON��NODE_OFF����Щ����Ҫ��Ϣ��ֻҪ���б��ϵĽڵ㶼Ҫ����֪ͨ������Ҫȷ���յ������Բ��ö�ʱ���ͻ���
 * @param drum Ͱ��
 * @return 0��ʾ���ͳɹ�������ʧ��
 */
int sendNodeOnOffBroadInfo(uint64 *time,enum RealLabel type,uint8 drum);

/**
 * �㲥���ͽڵ�Һλ������Ϣ
 * @param time ʱ���ǩ
 * @param type ,RealLabelö�ٳ���,ȡֵΪalarmϵ��
 * @param drum Ͱ��
 * @param leveldiff Һλ��ֵ
 * @param 0��ʾ���ͳɹ�������ʧ��
 */
int sendAlarmBroadInfo(uint64 *time,enum RealLabel type,uint8 drum,uint32 leveldiff);

/**
 * ��ѯ��ʱ��ʵʱ�ڵ㣬�ҵ���ɾ��
 */
void *pollOverTimeRTNode();





#endif
