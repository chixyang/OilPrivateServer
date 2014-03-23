
#ifndef DBIO_H_
#define DBIO_H_

/**数据库操作函数声明**/

#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <time.h>
#include "private_dbpool.h"
#include "private_debug.h"

//用户信息枚举常量
enum UserDetail{
	USER_NONE,				//无信息
	USER_USERNAME,			//用户名
	USER_PASSWORD,			//用户密码
	USER_PHONE,		     	//用户电话
	USER_RNAME			    //用户真实姓名
};

//液位和温度标签枚举常量
enum LTLabel{
	LT_NONE,				//无信息
	L_FIRST,				//第一级液位信息
	L_SECOND,				//第二级液位信息
	L_THIRD,				//第三级液位信息
	T_FIRST,                //第一级温度信息
	T_SECOND,				//第二级温度信息
	T_THIRD,				//第三级温度信息
	LT_FIRST				//液位、温度第一级信息
};

//用户锁信息枚举常量
enum UserLock{
	LOCK_NONE,
	LOCK_USERLABEL,			//锁用户编号
	LOCK_USERNAME			//用户名
};

//液位和温度信息结构
typedef struct lt_info{
	uint64 time;           //当前时间
	uint8 drum;          //桶号
	union{
		uint16 level;      //液位
		float temp;        //温度
	}lt;
	uint16 num;			   //液位（温度）的数量
	struct lt_info *next;
}LTInfo; 

typedef struct ol_info{
	uint8 lock;				//锁编号
	char username[20];		//用户名
	uint64 open_time;		//开锁时间
	uint64 close_time;		//关锁时间
	struct ol_info *next;
}OLInfo;



/*
 * 以秒为单位获取当前系统时间
 * @return uint64 64位秒数
 */
uint64 getCurrentTimeSeconds();

/**
 * 添加用户详细信息
 * @param username 要添加的用户名
 * @param passwrod 用户密码
 * @param phone 用户手机号
 * @param rname 用户真实姓名
 * @return 0 添加成功，其他失败
 */
int addUserDetail(char *username,char *passwrod,uint32 phone,char *rname);

/**
 * 获取用户单个信息,获取的信息空间需要显式释放,如果获取的是int，则需要另外调用atoi函数转换
 * @param username 要获取的用户名
 * @param type 获取信息的类型，为UserDetail常量
 * @return 返回获取的信息，需要显式释放，NULL表示未获取信息
 */
char *getUserDetail(char *username,enum UserDetail type);

/**
 * 更新用户信息
 * @param username 要更新的用户
 * @param type 要更新的用户属性，为UserDetail常量
 * @param value 更新后的内容，int的话就强制转为指针,譬如a为整数，则传参为（void *）a
 * @return 0 更新成功，其他失败
 */
int updateUserDetail(char *username,enum UserDetail type,void *value);

/**
 * 添加Level和Temp数据,桶的编号不能从0算起
 * @param info 添加的液位或温度结构
 * @param type 要添加的表属性，为LTLabel枚举常量
 * @return 0 表示添加成功，其他失败
 */
int addLTInfo(LTInfo *info,enum LTLabel type);

/**
 * 获取单个油桶的液位或温度信息（多个油桶的话则由接收协议判断并多次调用该函数）
 * @param starttime 查询开始时间
 * @param drum 油桶编号
 * @param num 查询数量
 * @param type 查询的表属性，为LTLabel枚举常量
 * @return LTInfo列表，NULL表示未查到
 */
LTInfo *getLTInfo(uint64 starttime,uint8 drum,uint8 num,enum LTLabel type);

/**
 * 获取第一级液位信息
 * @param starttime 开始时间
 * @param drum 油桶号
 * @param num 要获取的数量
 * @return 液位信息列表
 */
LTInfo *getLevelFirstInfo(uint64 starttime,uint8 drum,uint8 num);

/**
 * 获取第二三级液位信息
 * @param starttime 开始时间
 * @param drum 油桶号
 * @param num 要获取的数量
 * @param type 标记是第二级还是第三级信息
 * @return 液位信息列表
 */
LTInfo *getLevelSTInfo(uint64 starttime,uint8 drum,uint8 num,enum LTLabel type);

/**
 * 获取第一级温度信息
 * @param starttime 开始时间
 * @param drum 油桶号
 * @param num 要获取的数量
 * @return 液位信息列表
 */
LTInfo *getTempFirstInfo(uint64 starttime,uint8 drum,uint8 num);

/**
 * 获取第二三级温度信息
 * @param starttime 开始时间
 * @param drum 油桶号
 * @param num 要获取的数量
 * @param type 标记是第二级还是第三级信息
 * @return 液位信息列表
 */
LTInfo *getTempSTInfo(uint64 starttime,uint8 drum,uint8 num,enum LTLabel type);

/**
 * 添加用户锁信息
 * @param lock 锁编号
 * @param label 用户在锁上的编号
 * @param username 用户名
 * @return 0 添加成功，其他失败
 */
int addUserLockInfo(uint8 lock,char *label,char *username);

/**
 * 通过锁编号和用户名(锁用户编号)获取锁信息记录id,user既可能为username也可能为userlabel
 * @param lock 锁编号
 * @param user username或者user_label
 * @param type UserLock枚举常量，标记是user username还是user_label
 * @return 返回userlock_id，0则表示查询失败
 */
uint16 getIDbyUserLock(uint8 lock,char *user,enum UserLock type);

/**
 * 添加开锁信息
 * @param userlock_id 用户锁项编号
 * @param lock 锁编号
 * @param open_time 开锁时间
 * @return 0 添加成功，其他失败
 */
int addOpenLockInfo(uint16 userlock_id,uint8 lock,uint64 open_time);

/**
 * 更新开锁信息
 * @param userlock_id 用户锁项编号
 * @param open_time 开锁时间
 * @param close_time 要更新的关锁时间
 * @return 0 更新成功，其他失败
 */
int updateOpenLockInfo(uint16 userlock_id,uint64 open_time,uint64 close_time);

/**
 * 通过用户锁信息id获取开锁信息
 * @param userlock_id 用户锁信息id
 * @param starttime 查询起始时间
 * @param num 要查询的数量
 * @return 开锁信息列表，表中lock和username项为空
 */
OLInfo *getOLInfoByID(uint16 userlock_id,uint64 starttime,uint8 num);

/**
 * 通过锁号查锁信息
 * @param lock 锁编号
 * @param starttime 查询开始时间
 * @param num 查询数量
 * @return 开锁信息列表，表中lock项为空,，协议中当lock不为0，openuser为NULL时候
 */
OLInfo *getOLInfoByLock(uint8 lock,uint64 starttime,uint8 num);

/**
 * 仅按照时间顺序获取一定数量的开锁记录,协议中当lock为全0，openuser为null时候
 * @param starttime 查询开始时间
 * @param num 要查询的数量
 * @return 开锁信息列表，表中信息全
 */
OLInfo *getOLInfoByTime(uint64 starttime,uint8 num);

/**
 * 仅按照时间顺序和用户名获取一定数量的开锁记录,协议中当lock为全0，openuser不为null时候
 * @param starttime 查询开始时间
 * @param num 查询数量
 * @param username 查询的用户名
 * @return 开锁信息列表，表中username为空
 */
OLInfo *getOLInfoByTimeUser(uint64 starttime,uint8 num,char *username);

/**
 * 释放LTInfo列表
 * @param 要释放的列表
 */
 void freeLTInfoList(LTInfo *info);
 
 /**
  * 释放OLInfo列表
  * @param 要释放的列表
  */
 void freeOLInfoList(OLInfo *info);















#endif
