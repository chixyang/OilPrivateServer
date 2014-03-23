
//ʵʱ��Ϣ�����ݽṹ�ʹ�����
#include "private_realtime.h"

#define OverTimeRTPollTime 180  //��ʱ�ڵ���ѯɾ��ʱ�䣬�����ӣ���λ��

//��Ͱ����
const uint8 NumofDrums = 6;

RealSendNode *RealSendList = NULL;
pthread_mutex_t send_lock = PTHREAD_MUTEX_INITIALIZER;

//ǰ��һ��������Һλֵ
uint16 *former_first_level;

//ǰ�ڶ���������Һλֵ
uint16 *former_second_level;

//ǰ������������Һλֵ
uint16 *former_third_level;                 //��������ֻ�е��̲߳��������Բ��ü���

//��һ��Һλ�ڵ��б�
LevelFirstNode *LevelFirstList = NULL;
//�ڶ���Һλ�ڵ��б�
LevelSecondNode *LevelSecondList = NULL;
//������Һλ�ڵ��б�
LevelThirdNode *LevelThirdList = NULL;

//��һ���¶Ƚڵ��б�
TempFirstNode *TempFirstList = NULL;
//�ڶ���Һλ�ڵ��б�
TempSecondNode *TempSecondList = NULL;
//������Һλ�ڵ��б�
TempThirdNode *TempThirdList = NULL;

//��ʼ��ʵʱ��Ϣ�ṹ�б�
void initRealLTList()
{
	//��ʼ��ʵʱ�����б�
	RealSendList = NULL;
	//��ʼ����������Һλ���ݺ�
	former_first_level = (uint16 *)malloc(sizeof(uint16) * NumofDrums);
	memset(former_first_level,0,sizeof(uint16) * NumofDrums);
	former_second_level= (uint16 *)malloc(sizeof(uint16) * NumofDrums);
	memset(former_second_level,0,sizeof(uint16) * NumofDrums);
	former_third_level = (uint16 *)malloc(sizeof(uint16) * NumofDrums);
	memset(former_third_level,0,sizeof(uint16) * NumofDrums);
	
	//�׽ڵ�
	LevelFirstList = (LevelFirstNode *)malloc(sizeof(LevelFirstNode));
	memset(LevelFirstList,0,sizeof(LevelFirstNode));
	LevelFirstList->drum = 1;								//Ͱ�Ŵ�1��ʼ
	pthread_mutex_init(&(LevelFirstList->lock),NULL);		//��ʼ����
	LevelFirstNode *lf = LevelFirstList;
	//�׽ڵ�
	LevelSecondList = (LevelSecondNode *)malloc(sizeof(LevelSecondNode));
	memset(LevelSecondList,0,sizeof(LevelSecondNode));
	LevelSecondList->drum = 1;
	pthread_mutex_init(&(LevelSecondList->lock),NULL);
	LevelSecondNode *ls = LevelSecondList;
	//�׽ڵ�
	LevelThirdList = (LevelThirdNode *)malloc(sizeof(LevelThirdNode));
	memset(LevelThirdList,0,sizeof(LevelThirdNode));
	LevelThirdList->drum = 1;
	pthread_mutex_init(&(LevelThirdList->lock),NULL);
	LevelThirdNode *lt = LevelThirdList;
	//�׽ڵ�
	TempFirstList = (TempFirstNode *)malloc(sizeof(TempFirstNode));
	memset(TempFirstList,0,sizeof(TempFirstNode));
	TempFirstList->drum = 1;
	pthread_mutex_init(&(TempFirstList->lock),NULL);
	TempFirstNode *tf = TempFirstList;
	//�׽ڵ�
	TempSecondList = (TempSecondNode *)malloc(sizeof(TempSecondNode));
	memset(TempSecondList,0,sizeof(TempSecondNode));
	TempSecondList->drum = 1;
	pthread_mutex_init(&(TempSecondList->lock),NULL);
	TempSecondNode *ts = TempSecondList;
	//�׽ڵ�
	TempThirdList = (TempThirdNode *)malloc(sizeof(TempThirdNode));
	memset(TempThirdList,0,sizeof(TempThirdNode));
	TempThirdList->drum = 1;
	pthread_mutex_init(&(TempThirdList->lock),NULL);
	TempThirdNode *tt = TempThirdList;
	//ѭ����ʼ�������ڵ�
	for(int i = 1;i < NumofDrums;i++)
	{
		lf->next = (LevelFirstNode *)malloc(sizeof(LevelFirstNode));
		lf = lf->next;
		memset(lf,0,sizeof(LevelFirstNode));
		lf->drum = i + 1;
		pthread_mutex_init(&(lf->lock),NULL);
	
		ls->next = (LevelSecondNode *)malloc(sizeof(LevelSecondNode));
		ls = ls->next;
		memset(ls,0,sizeof(LevelSecondNode));
		ls->drum = i + 1;
		pthread_mutex_init(&(ls->lock),NULL);
	
		lt->next = (LevelThirdNode *)malloc(sizeof(LevelThirdNode));
		lt = lt->next;
		memset(lt,0,sizeof(LevelThirdNode));
		lt->drum = i + 1;
		pthread_mutex_init(&(lt->lock),NULL);
	
		tf->next = (TempFirstNode *)malloc(sizeof(TempFirstNode));
		tf = tf->next;
		memset(tf,0,sizeof(TempFirstNode));
		tf->drum = i + 1;
		pthread_mutex_init(&(tf->lock),NULL);
	
		ts->next = (TempSecondNode *)malloc(sizeof(TempSecondNode));
		ts = ts->next;
		memset(ts,0,sizeof(TempSecondNode));
		ts->drum = i + 1;
		pthread_mutex_init(&(ts->lock),NULL);
	
		tt->next = (TempThirdNode *)malloc(sizeof(TempThirdNode));
		tt = tt->next;
		memset(tt,0,sizeof(TempThirdNode));
		tt->drum = i + 1;
		pthread_mutex_init(&(tt->lock),NULL);
	}
}

//�յ���������Ϣ,����ȡ��ַ��typeֻ������first����
int addNewLTInfo(enum LTLabel type,void *info,uint8 drum)
{
	if((info == NULL) || (drum == (uint8)0) || (drum >= NumofDrums))
		return -1;
	
	switch(type)
	{
		case L_FIRST: //�յ�Һλ��Ϣ
		{	LevelFirstNode *lf = LevelFirstList;
			int step = 0;  //����ֱ�ӵ�������������ڵ�ṹ
			while(lf != NULL)
			{
				if(lf->drum == drum)  //�ҵ���ӵĽڵ㣬��Щ�ڵ㶼���ᱻɾ��������drum����ı䣬���ԣ���lock���Բ��ð�����仰
				{	
					if(my_mutex_lock(&lf->lock)) //�������Ҫ������δ��ȡ����
						return -1;
					uint64 curtime = getCurrentTimeSeconds();//��ȡ��ǰʱ��
					uint16 level = *(uint16 *)info;  //��ȡҺλ��Ϣ
					LTInfo *ltinfo = NULL;
					int alarm = 0;
					uint16 first_leveldiff = 0;
					if(lf->time != (uint64)0)  //�����Ϊϵͳ��һ�ν�������
					{
						//�ж��Ƿ���Ҫ����
						if((former_first_level[drum - 1] > level) && ((first_leveldiff=(former_first_level[drum - 1] - level)) >= LevelFirstDiff))
						{
							//debug("first level alarm\n");
							//����
							sendAlarmBroadInfo(&curtime,REAL_ALARM_FISRT,drum,first_leveldiff);
							alarm = 1;
						}
						
						//����ϵͳ��һ�ν������ݵĻ���Ҫ��ԭ����ֵ�������ݿ���
						ltinfo = (LTInfo *)malloc(sizeof(LTInfo));
						memset(ltinfo,0,sizeof(LTInfo));
						ltinfo->time = lf->time;
						ltinfo->drum = lf->drum;
						ltinfo->lt.level = lf->lt.level;
						if(addLTInfo(ltinfo,L_FIRST))
						{
							debug("add ltinfo first level error\n");
							free(ltinfo);
							pthread_mutex_unlock(&lf->lock);
							return -1;
						}
					}
					former_first_level[drum - 1] = level;
					//��ʼ�޸ĵ�һ���ڵ���Ϣ
					lf->time = curtime;
					lf->lt.level = level;
					if(lf->active == 0)
					{
						lf->active = 1;
						/******��ͻ��˷�����Ϣ֪ͨ�ڵ�ɹ�����Э�飬��ѯʵʱ�����б�Ȼ����*******/
						//debug("******node add in success*********\n");
						sendNodeOnOffBroadInfo(&curtime,REAL_LEVEL_NODE_ON,drum);
					}
					
					pthread_mutex_unlock(&(lf->lock));
					lf = NULL;  //�Է�����
					
					/**�����ҵ��ڶ����ڵ㲢�޸���Ϣ**/
					LevelSecondNode *ls = LevelSecondList;
					for(int i = 0; i < step; i++)  //���ݳ�ʼ��ʱ�Ľṹ��֪��������Ͱ����һһ��Ӧ��
						ls = ls->next;
					if((ls == NULL) || (ls->drum != drum))
					{
						debug("can not find second level by first\n");
						free(ltinfo);
						return -1;
					}
					//�ҵ��ڶ����ڵ㣬�Ƚ�ʱ�䣬��ӽڵ���Ϣ
					if(my_mutex_lock(&(ls->lock)))
					{
						free(ltinfo);
						return -1;
					}
					if((curtime / FSTTOSEC) == (ls->time / FSTTOSEC))  //��ͬһ��ʱ�����,time��ls->time����ÿСʱ������ֵ�϶���ͬ
					{
						//ʱ��time�����޸�
						ls->lt.sum_level += level;
						ls->num++;
						pthread_mutex_unlock(&(ls->lock));
					}
					else				//����ͬһ��ʱ�����
					{
						if(ls->num != 0)   //�ǵ�һ�δ���ֵ,��һ�ε�ʱ��numҲΪ0
						{
							uint16 second_leveldiff = 0;
							//�ж��Ƿ���Ҫ����
							if((!alarm) && (former_second_level[drum-1] > level) && ((second_leveldiff=(former_second_level[drum-1] - level)) >= LevelSecondDiff))
							{
								debug("second level alarm\n");
								//����
								sendAlarmBroadInfo(&curtime,REAL_ALARM_SECOND,drum,second_leveldiff);
								alarm = 1;
							}
							
							//�Ƚ�ԭʱ��δ������ݿ���
							ltinfo->time = ls->time;					//drumǰ���Ѿ���ֵ����
							ltinfo->lt.level = (uint16)((ls->lt.sum_level) / (ls->num));    //�������ݿ����ƽ��ֵ
							ltinfo->num = ls->num;
							if(addLTInfo(ltinfo,L_SECOND))
							{
								pthread_mutex_unlock(&(ls->lock));
								debug("add ltinfo error");
								free(ltinfo);
								return -1;
							}
						}
						former_second_level[drum-1] = level;
						//Ȼ����ö����ڵ㸳��ֵ
						uint32 sum_level = ls->lt.sum_level;
						uint16 num = ls->num;
						ls->time = curtime;
						ls->lt.sum_level = level;
						ls->num = 1;
						pthread_mutex_unlock(&(ls->lock));
						ls = NULL;
						
						/** �������ڵ㱻�������ݿ��ͬʱ���ýڵ����������ڵ��� **/
						LevelThirdNode *lt = LevelThirdList;
						for(int i = 0; i < step; i++)  //�ҵ������ڵ�
							lt = lt->next;
						if((lt == NULL) || (lt->drum != drum))
						{
							debug("can not find third level by second\n");
							free(ltinfo);
							return -1;
						}
						if(my_mutex_lock(&(lt->lock)))
						{
							free(ltinfo);
							return -1;
						}
						if((curtime / SECTOTHD) == (lt->time / SECTOTHD)) //��������ڵ���ͬһ���趨��ʱ�����
						{
							lt->lt.sum_level += sum_level;
							lt->num += num;
							pthread_mutex_unlock(&(lt->lock));
						}
						else    //����һ��ʱ�����
						{
							if(lt->num != 0)
							{
								uint16 third_leveldiff = 0;
								//�ж��Ƿ���Ҫ����
								if((!alarm) && (former_third_level[drum-1] > level) && ((third_leveldiff=(former_third_level[drum-1] - level)) >= LevelThirdDiff))
								{//����
									debug("third level alarm\n");
									sendAlarmBroadInfo(&curtime,REAL_ALARM_THIRD,drum,third_leveldiff);
								}
								ltinfo->time = lt->time;					//drumǰ���Ѿ���ֵ����
								ltinfo->lt.level = (uint16)((lt->lt.sum_level) / (lt->num));    //�������ݿ����ƽ��ֵ
								ltinfo->num = lt->num;
								if(addLTInfo(ltinfo,L_THIRD))
								{
									pthread_mutex_unlock(&(lt->lock));
									debug("add ltinfo error");
									free(ltinfo);
									return -1;
								}
							}
							former_third_level[drum-1] = level;
							//�������ڵ㸳��ֵ
							lt->time = curtime;
							lt->lt.sum_level = sum_level;
							lt->num = num;
							pthread_mutex_unlock(&(lt->lock));
						}
					}
					if(ltinfo)
						free(ltinfo);
					return 0;
				}
				lf = lf->next;
				step++;
			}
		}
			break;
		case T_FIRST: //�յ��¶���Ϣ
		{//	debug("in temp first add\n");
			TempFirstNode *tf = TempFirstList;
			int step = 0;  //����ֱ�ӵ�������������ڵ�ṹ
			while(tf != NULL)
			{
				if(tf->drum == drum)  //�ҵ���ӵĽڵ㣬��Щ�ڵ㶼���ᱻɾ��������drum����ı䣬���ԣ���lock���Բ��ð�����仰
				{
					if(my_mutex_lock(&(tf->lock)))
						return -1;
					uint64 curtime = getCurrentTimeSeconds(); //��ȡ��ǰʱ��
					float temp = *(float *)info;  //��ȡ�¶���Ϣ
					LTInfo *ltinfo = NULL;
					if(tf->time != (uint64)0)  //�����Ϊϵͳ��һ�ν�������
					{
						//�¶Ȳ���Ҫ��������ԭ����ֵ�������ݿ���
						ltinfo = (LTInfo *)malloc(sizeof(LTInfo));
						memset(ltinfo,0,sizeof(LTInfo));
						ltinfo->time = tf->time;
						ltinfo->drum = tf->drum;
						ltinfo->lt.temp = tf->lt.temp;
						if(addLTInfo(ltinfo,T_FIRST))
						{
							debug("add ltinfo error");
							free(ltinfo);
							return -1;
						}
					}
					//��ʼ�޸ĵ�һ���ڵ���Ϣ
					tf->time = curtime;
					tf->lt.temp = temp;
					if(tf->active == 0)
					{
						tf->active = 1;
						/******��ͻ��˷�����Ϣ֪ͨ�ڵ�ɹ�����Э�飬��ѯʵʱ�����б�Ȼ����*******/
						sendNodeOnOffBroadInfo(&curtime,REAL_TEMP_NODE_ON,drum);
					}
					pthread_mutex_unlock(&(tf->lock));
					tf = NULL;  //�Է�����
					
					/**�����ҵ��ڶ����ڵ㲢�޸���Ϣ**/
					TempSecondNode *ts = TempSecondList;
					for(int i = 0; i < step; i++)  //���ݳ�ʼ��ʱ�Ľṹ��֪��������Ͱ����һһ��Ӧ��
						ts = ts->next;
					if((ts == NULL) || (ts->drum != drum))
					{
						debug("can not find second level by first\n");
						free(ltinfo);
						return -1;
					}
					//�ҵ��ڶ����ڵ㣬�Ƚ�ʱ�䣬��ӽڵ���Ϣ
					if(my_mutex_lock(&(ts->lock)))
					{
						free(ltinfo);
						return -1;
					}
					if((curtime / FSTTOSEC) ==(ts->time / FSTTOSEC))  //��ͬһ��ʱ�����,timeһ����ts->time�󣬵�һ�εĻ�ts->time Ϊ0����ֵһ������FSTTOSEC�������⼸����벻ִ��
					{
						//ʱ��time�����޸�
						ts->lt.sum_temp += temp;
						ts->num++;
						pthread_mutex_unlock(&(ts->lock));
					}
					else				//����ͬһ��ʱ�����
					{
						if(ts->num != 0)
						{
							//�Ƚ�ԭʱ��δ������ݿ���
							ltinfo->time = ts->time;					//drumǰ���Ѿ���ֵ����
							ltinfo->lt.temp = (ts->lt.sum_temp) / (ts->num);    //�������ݿ����ƽ��ֵ
							ltinfo->num = ts->num;
							if(addLTInfo(ltinfo,T_SECOND))
							{
								pthread_mutex_unlock(&(ts->lock));
								debug("add ltinfo error");
								free(ltinfo);
								return -1;
							}
						}
						//Ȼ����ö����ڵ㸳��ֵ
						float sum_temp = ts->lt.sum_temp;
						uint16 num = ts->num;
						ts->time = curtime;
						ts->lt.sum_temp = temp;
						ts->num = 1;
						pthread_mutex_unlock(&(ts->lock));
						ts = NULL;
						
						/** �������ڵ㱻�������ݿ��ͬʱ���ýڵ����������ڵ��� **/
						TempThirdNode *tt = TempThirdList;
						for(int i = 0; i < step; i++)  //�ҵ������ڵ�
							tt = tt->next;
						if((tt == NULL) || (tt->drum != drum))
						{
							debug("can not find third level by second\n");
							free(ltinfo);
							return -1;
						}
						if(my_mutex_lock(&(tt->lock)))
						{
							free(ltinfo);
							return -1;
						}
						if((curtime / SECTOTHD) ==(tt->time / SECTOTHD)) //��������ڵ���ͬһ���趨��ʱ�����
						{
							tt->lt.sum_temp += sum_temp;
							tt->num += num;
							pthread_mutex_unlock(&(tt->lock));
						}
						else    //����һ��ʱ�����
						{
							if(tt->num != 0)
							{
								ltinfo->time = tt->time;					//drumǰ���Ѿ���ֵ����
								ltinfo->lt.temp = (tt->lt.sum_temp) / (tt->num);    //�������ݿ����ƽ��ֵ
								ltinfo->num = tt->num;
								if(addLTInfo(ltinfo,T_THIRD))
								{
									pthread_mutex_unlock(&(tt->lock));
									debug("add ltinfo error");
									free(ltinfo);
									return -1;
								}
							}
							//�������ڵ㸳��ֵ
							tt->time = curtime;
							tt->lt.sum_temp = sum_temp;
							tt->num = num;
							pthread_mutex_unlock(&(tt->lock));
						}
					}
					if(ltinfo)  //��Ϊ�����ͷ�
						free(ltinfo);
					return 0;
				}
				tf = tf->next;
				step++;
			}
		}
			break;
		default:
			return -1;
	}
	
	return -1;
}


//��ѯʵʱ������Ϣ,��drumΪ0ʱ����ʾȫ����ѯ����ѯ�����Ϊfirst ��һ�����ݣ����������ݲ�Ϊʵʱ����
RealInfo *getCurrentLTInfo(uint8 drum)
{
	//debug("in get current lt info\n");
	LevelFirstNode *lf = LevelFirstList;
	TempFirstNode *tf = TempFirstList;
	RealInfo *ri = NULL;
	RealInfo *node = NULL;
	
	//debug("drum = %d\n",drum);
	if(drum == 0) 
	{//ȫ����ѯ

		while((lf != NULL) && (tf != NULL))
		{
			if(my_mutex_lock(&(lf->lock)))
			{
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(my_mutex_lock(&(tf->lock)))
			{
				pthread_mutex_unlock(&(lf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			//debug("lf->drum : %d, lf->active : %d; tf->drum : %d,tf->active : %d\n",lf->drum,lf->active,tf->drum,tf->active);
			if(!((lf->active) || (tf->active)))	//���ٱ�֤����Ͱ����һ���ڵ��ڹ�����������Ϊ��ѯ���
			{
				pthread_mutex_unlock(&(lf->lock));   //��Ƕ���Ƿǳ�Σ�յ���Ϊ���������д�����ֻ�д˴��õ�����Ƕ�ף����԰�ȫ
				pthread_mutex_unlock(&(tf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(ri != NULL)
			{
				node->next = (RealInfo *)malloc(sizeof(RealInfo));
				node = node->next;
				memset(node,0,sizeof(RealInfo));
			}
			else
			{
				ri = (RealInfo *)malloc(sizeof(RealInfo));
				memset(ri,0,sizeof(RealInfo));
				node = ri;
			}
			node->drum = lf->drum;
			node->level = lf->lt.level;
			if(lf->active == 0)
				node->level = 0;
			node->temp = tf->lt.temp;
			if(tf->active == 0)
				node->temp == .0;
			
			pthread_mutex_unlock(&(tf->lock));
			pthread_mutex_unlock(&(lf->lock));
			//lf��tf��Ӧ��
			lf = lf->next;
			tf = tf->next;
		}

		//debug("geted real info\n");
		return ri;
	}
	else  //������ѯ
	{
		while((lf != NULL) && (tf != NULL))
		{
			if(my_mutex_lock(&(lf->lock)))
			{
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(my_mutex_lock(&(tf->lock)))
			{
				pthread_mutex_unlock(&(lf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if(!((lf->active) || (tf->active)))
			{
				pthread_mutex_unlock(&(tf->lock));
				pthread_mutex_unlock(&(lf->lock));
				lf = lf->next;
				tf = tf->next;
				continue;
			}
			if((lf->drum == drum) && (tf->drum == drum))
			{
				ri = (RealInfo *)malloc(sizeof(RealInfo));
				memset(ri,0,sizeof(RealInfo));
				ri->drum = drum;
				ri->level = lf->lt.level;
				if(lf->active == 0)
					ri->level = 0;
				ri->temp = tf->lt.temp;
				if(tf->active == 0)
					ri->temp == .0;

				pthread_mutex_unlock(&(tf->lock));
				pthread_mutex_unlock(&(lf->lock));
				break;
			}
			
			pthread_mutex_unlock(&(tf->lock));
			pthread_mutex_unlock(&(lf->lock));
			lf = lf->next;
			tf = tf->next;
		}
		//debug("get single realinfo\n");
		return ri;
	}
}

//�ͷ�RealInfo�б�
void freeRealInfoList(RealInfo *ri)
{
	if(ri != NULL)
	{
		RealInfo *tmp = ri->next;
		while(tmp != NULL)
		{
			ri->next = tmp->next;
			free(tmp);
			tmp = ri->next;
		}
		free(ri);
	}
}

//��ѯ�б�(�鿴�Ƿ��нڵ㳬ʱδ�յ�����),�̺߳���
void *pollDiedNode()
{
	while(1)
	{
		sleep(PollDiedTime);

		uint64 time = getCurrentTimeSeconds();
		//����һ��Һλ�ڵ�
		LevelFirstNode *lf = LevelFirstList;
		while(lf != NULL)
		{
			if(my_mutex_lock(&(lf->lock)))
			{
				lf = lf->next;
				continue;
			}
			if((lf->active == 1) && ((time - lf->time) >= ActiveDiff))  //�ڵ��Ѿ�died
			{
				lf->active = 0;
				/**��ͻ��˷�����Ϣ֪ͨ�ýڵ��Ѿ�died**/
				sendNodeOnOffBroadInfo(&time,REAL_LEVEL_NODE_OFF,lf->drum);
			}
			pthread_mutex_unlock(&(lf->lock));
			lf = lf->next;
		}
		//����һ���¶Ƚڵ�
		TempFirstNode *tf = TempFirstList;
		while(tf != NULL)
		{
			if(my_mutex_lock(&(tf->lock)))
			{
				tf = tf->next;
				continue;
			}
			if((tf->active == 1) && ((time - tf->time) >= ActiveDiff))  //�ڵ��Ѿ�died
			{
				tf->active = 0;
				/**��ͻ��˷�����Ϣ֪ͨ�ýڵ��Ѿ�died**/
				sendNodeOnOffBroadInfo(&time,REAL_TEMP_NODE_OFF,tf->drum);
			}
			pthread_mutex_unlock(&(tf->lock));
			tf = tf->next;
		}
	}
}

//����ʵʱ�����б�
int InRealSendList(char *username,uint8 drum,int fd,struct sockaddr_in *addr, enum RealLabel type)
{
	if((username == NULL) || (fd == 0) || (addr == NULL) || (type == REAL_NONE))
		return -1;
	
	//���жϸýڵ��Ƿ��Ѿ��������б���
	if(my_mutex_lock(&(send_lock)))
		return -1;
	RealSendNode *node = RealSendList, *preNode = NULL;
	while(node != NULL)
	{
		if(!memcmp(node->username,username,strlen(username))) //�Ѿ�����
		{
			//��������ַ�Ƿ���ͬ��Ҳ��ͬ�Ļ����ٽ����½ڵ㣬��ͬ��ɾ����ǰ�ڵ㲢�����½ڵ�
			if((node->sendaddr.sin_addr.s_addr == addr->sin_addr.s_addr) && (node->sendaddr.sin_port == addr->sin_port))
			{	debug("duplicate node in real send list\n");
				pthread_mutex_unlock(&(send_lock));
				RTSendAction(node);
				return 0;
			}
			else
			{
				//ɾ��ԭ���ڵ�,���ýڵ���б���ɾ����������ԭ���ڵ��timerֵ���̻߳��Զ�free�ýڵ�
				if(node == RealSendList) //Ҫɾ������ͷ���
					RealSendList = node->next;
				else //��ͨ�ڵ�
					preNode->next = node->next;
			}
			break;
		}
		
		preNode = node;
		node = preNode->next;
	}
	pthread_mutex_unlock(&(send_lock));
	//����timer��־��ʹ�߳�ɾ���ýڵ㣬���������������Ƕ��
	if(node != NULL)
	{
		if(my_mutex_lock(&(node->lock)))
		{
			node->timer = 0;   //��ʹδ��ȡ����ҲҪǿ��ת������Ϊ�ýڵ��Ѿ�ɾ��
			return -1;
		}
		node->timer = 0;
		pthread_mutex_unlock(&(node->lock));
	}
	
	//�½��ڵ�(�ڵ�δ�ҵ����߸ոձ�ɾ��)
	RealSendNode *rsn = (RealSendNode *)malloc(sizeof(RealSendNode));
	memset(rsn,0,sizeof(RealSendNode));
	memcpy(rsn->username,username,strlen(username));  //�û�����ֵ
	rsn->drum = drum;
	rsn->fd = fd;				//�׽����ļ�
	memcpy(&(rsn->sendaddr),addr,sizeof(struct sockaddr_in));  //�û���ַ
	rsn->type = type;				//��������
	rsn->timer = REAL_SEND_TIMER;
	pthread_mutex_init(&(rsn->lock),NULL);
	
	//debug("before add in real send list\n");
	//�����б�
	if(my_mutex_lock(&(send_lock)))
	{
		pthread_mutex_destroy(&(rsn->lock));
		free(rsn);
		return -1;
	}
	if(RealSendList == NULL)  //�б���ʼΪ��
		RealSendList = rsn;
	else   //�����Ϊͷ���
	{
		rsn->next = RealSendList;
		RealSendList = rsn;
	}
	pthread_mutex_unlock(&(send_lock));
	
	//��һ���߳�ִ�иýڵ�����(����Ҫ��)
//	pthread_t tid;
//	pthread_create(&tid,NULL,RTSendAction,(void *)rsn);   //�����߳�ѭ��ִ���ط�����
//	rsn->tid = tid;
	rsn->tid = 0;
	//pthread_detach(tid);		//��Ҫ�����̷߳��룬��ΪҪ�ȴ����߳����

	debug("after add in real time list\n");
	return 0;
}

//ʵʱ�����б������Ҫ�ͻ��˷���Ϣ�Ż�ɾ����
int QuitRealSendList(char *username,uint8 drum,enum RealLabel type)
{
	if((username == NULL))
		return -1;
	debug("in quit real send,drum : %d\n",drum);
	//�ҵ��ڵ�
	if(my_mutex_lock(&(send_lock)))
		return -1;
	RealSendNode *rsn = RealSendList, *preRsn = NULL;
	while(rsn != NULL)
	{
		if((!memcmp(username,rsn->username,strlen(username))) && (drum == rsn->drum) && (type == rsn->type))
			break;
		preRsn = rsn;
		rsn = preRsn->next;
	}
	 
	if(rsn == NULL) //Ҫôû�ҵ���Ҫô�ڵ��б�Ϊ��
	{
		pthread_mutex_unlock(&(send_lock));
		return 0;
	}
	else  //�п�����ͷ���,Ҳ�п�������ͨ�ڵ�
	{
		if(preRsn == NULL)   //ͷ���
			RealSendList = rsn->next;
		else
			preRsn->next = rsn->next;
	}
		
	pthread_mutex_unlock(&(send_lock));
	
	//����rsn��timerΪ0�����߳��Զ�free�ýڵ�
	if(my_mutex_lock(&(rsn->lock)))
	{
		rsn->timer = 0;  //ǿ��ת������Ϊ�Ѿ����б�ɾ��
		return 0;  //��ʾ�Ѿ��ɹ�
	}
	rsn->timer = 0;
	pthread_mutex_unlock(&(rsn->lock));
	
	return 0;
}

//�̺߳�����ʵʱ��Ϣ��ʱ���ͺ���
void *RTSendAction(void *arg)
{
	RealSendNode *rsn = (RealSendNode *)arg;
	//while(1)
	{
		//debug("real time send poll once\n");
		if(my_mutex_lock(&(rsn->lock)))
		{
			debug("can not get lock\n");
	//		continue;
		}
		if(rsn->timer <= 0)
		{
			debug("timer <= 0\n");
			pthread_mutex_unlock(&(rsn->lock));
			sleep(REAL_SEND_PERIOD);
	//		continue;
		}
		RealInfo *ri = getCurrentLTInfo(rsn->drum);  //��ȡʵʱ��Ϣ�б�
		if(ri == NULL)
		{
			//debug("ri == NULL");
			pthread_mutex_unlock(&(rsn->lock));
	//		continue;
		}
		uint16 len = 0;
		char *buf = getRealLTInfo(ri,rsn->username,&len);   //��ȡ����Э��
		freeRealInfoList(ri);
		if(buf == NULL)
		{
			debug("buf == NULL");
			pthread_mutex_unlock(&(rsn->lock));
	//		continue;
		}
	
	//	int sendlabel = rsn->timer;
	//	rsn->timer--;
	//	debug("rsn timer :%d\n",rsn->timer);
		pthread_mutex_unlock(&(rsn->lock));
		//��Ϊ��ʼ���ն���һ���̲߳���rsn����timer����Ķ���������fd��sendaddr����ı䣬�������������û������
	//ʵʱ��Ϣ���ٶ�ʱ����
	//	if(sendlabel == REAL_SEND_TIMER)
			InSendQueue(rsn->fd,&(rsn->sendaddr),buf,len);  //����
		//debug("real send once ,length: %d  ret : %d\n",len,ret);
		free(buf);
	
	//	sleep(REAL_SEND_PERIOD); //����
	}
	
//	pthread_exit("real time send quit");
}

//��ѯ��ʱ��ʵʱ�ڵ㣬�ҵ�ɾ��
void *pollOverTimeRTNode()
{
	while(1)
	{
		if(my_mutex_lock(&send_lock))
		{
			sleep(OverTimeRTPollTime);  //��ȡ����������������
			continue;
		}
		RealSendNode *rsn = RealSendList, *preRsn = NULL;
		
		while(rsn)
		{
			if(my_mutex_lock(&(rsn->lock)))
			{
				preRsn = rsn;
				rsn = preRsn->next;
				continue;
			}
			if(rsn->timer <= 0) //��Ҫ���б���ɾ���ýڵ�
			{
				if(rsn == RealSendList) //��һ���ڵ�
					RealSendList = rsn->next;
				else
					preRsn->next = rsn->next;
				
				pthread_mutex_unlock(&(rsn->lock));
				//void *status;
				//pthread_join(rsn->tid,&status); //�ȴ��������߳̽���
				RealSendNode *tmp = rsn->next;
				//�߳̽������ͷŸýڵ���Ϣ
				pthread_mutex_destroy(&rsn->lock);
				free(rsn);
				rsn = tmp;
				continue;
			}
			pthread_mutex_unlock(&(rsn->lock));
			preRsn = rsn;
			rsn = preRsn->next;
		}
		pthread_mutex_unlock(&send_lock);

		sleep(OverTimeRTPollTime);
	}
}

//�ҵ�ʵʱ�����б��еĽڵ㣬��ͨ�����б���Ϣ�ͷ�����Ϣ���뵽�����б�typeΪreallabelö�ٳ�����Ŀǰȡֵ��ΧΪALARM_FISRT��ALARM_SECOND��ALARM_THIRD��NODE_ON��NODE_OFF
int sendNodeOnOffBroadInfo(uint64 *time,enum RealLabel type,uint8 drum)
{
	//������ɵ�Э��Ϊ��
	if((time == NULL) || (drum == (uint8)0))
		return -1;
	
	char resend_type = RESEND_NONE;
	char *buf = NULL;
	uint16 len = 0;
	switch(type)
	{
		case REAL_LEVEL_NODE_ON:
			resend_type = RESEND_LEVEL_NODEON;
			break;
		case REAL_LEVEL_NODE_OFF:
			resend_type = RESEND_LEVEL_NODEOFF;
			break;
		case REAL_TEMP_NODE_ON:
			resend_type = RESEND_TEMP_NODEON;
			break;
		case REAL_TEMP_NODE_OFF:
			resend_type = RESEND_TEMP_NODEOFF;
			break;
		default:
			return -1;
	}
	if(my_mutex_lock(&send_lock))
		return -1;
	RealSendNode *rsn = RealSendList;
	//Ѱ�ҽڵ㷢��
	while(rsn != NULL)
	{
		buf = getNodeOnOffReport(rsn->username,drum,time,type,&len);
		//���нڵ㶼���ͣ�����Ҫ��֤���ͳɹ�������֤ʧ���ط�
		InTimelyReSendQueue(rsn->fd,&(rsn->sendaddr),buf,len,resend_type,drum);   //���͵���ʱ���ͺ�������ȷ����Щ��Ҫ��Ϣ�����ı�����
		free(buf);
		len = 0;
		rsn = rsn->next;
	}
	pthread_mutex_unlock(&send_lock);
	return 0;
}


//���ͽڵ�Һλ������Ϣ
int sendAlarmBroadInfo(uint64 *time,enum RealLabel type,uint8 drum,uint32 leveldiff)
{
	//������ɵ�Э��Ϊ��
	if((time == NULL) || (drum == (uint8)0) || (leveldiff == (uint32)0))
		return -1;
	
	char resend_type = RESEND_NONE;
	char *buf = NULL;
	uint16 len = 0;
	switch(type)
	{
		//������Ϣ���ǹ㲥��Ϣ
		case REAL_ALARM_FISRT:
		case REAL_ALARM_SECOND:
		case REAL_ALARM_THIRD:
			resend_type = RESEND_ALARM;
			break;
		default:
			return -1;
	}

	if(my_mutex_lock(&send_lock))
		return -1;
	RealSendNode *rsn = RealSendList;
	//Ѱ�ҽڵ㷢��
	while(rsn != NULL)
	{
		buf = getAlarmInfo(rsn->username,drum,time,type,leveldiff,&len);
		//���нڵ㶼���ͣ�����Ҫ��֤���ͳɹ�������֤ʧ���ط�
		InTimelyReSendQueue(rsn->fd,&(rsn->sendaddr),buf,len,resend_type,drum);   //���͵���ʱ���ͺ�������ȷ����Щ��Ҫ��Ϣ�����ı�����
		free(buf);
		len = 0;
		rsn = rsn->next;
	}
	pthread_mutex_unlock(&send_lock);
	return 0;
}



