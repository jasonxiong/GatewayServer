
#ifndef __MSG_STATISTIC_HPP__
#define __MSG_STATISTIC_HPP__

#include "Statistic.hpp"
#include "SingletonTemplate.hpp"

const int MAX_STAT_MSG_NUM = 128; //最多统计的消息种类数

//!用于通过MsgID方便定位到在CStatistic中的Section索引
typedef struct tagStatMsgInfo
{
	int m_iMsgID; //!<消息ID
	int m_iMsgIndex; //!<消息在CStaitistic中的Section索引
} TStatMsgInfo;

typedef enum enmStatMsgResult
{
	ESMR_SUCCEED = 0, //!<消息处理成功
	ESMR_FAILED = 1, //!<消息处理失败
	ESMR_TIMEOUT = 2, //!<消息处理超时
} ENMSTATMSGRESULT;

/**
*@brief 消息需要统计的统计项
*
*	
*/
typedef enum enmStatMsgItemIndex
{
	ESMI_SUCCESS_MSG_NUM_IDX = 0, //!<消息处理成功的个数
	ESMI_FAILED_MSG_NUM_IDX, //!<消息处理失败的个数 
	ESMI_TIMEOUT_MSG_NUM_IDX, //!<消息处理超时的个数
	ESMI_SUM_PROCESSTIME_IDX, //!<总处理耗时
	ESMI_MAX_PROCESSTIME_IDX, //!<最大处理耗时
	ESMI_AVA_PROCESSTIME_IDX, //!<平均处理耗时
	ESMI_MAX_ITEM_NUM //!<总共需要统计的统计项个数，保证这个值不会超过Section能容纳的Item最大值
} ENMSTATMSGITEMINDEX;

extern const char* g_apszMsgItemName[ESMI_MAX_ITEM_NUM];

class CMsgStatistic
{
public:
	CMsgStatistic();
	~CMsgStatistic();

public:
	/**
	*初始化，在初始化时会分配内存给CStatistic类中的Section
	*@param[in] pszStatPath 统计文件路径，默认是../stat/
	*@param[in] pszStatFileName 统计文件名，默认是s
	*@return 0 success
	*/
	int Initialize(const char* pszStatPath = NULL, const char* pszStatFileName = NULL);

	//!添加统计信息
	int AddMsgStat(int iMsgID, short shResult, timeval tvProcessTime);

	//!打印统计信息
	void Print();

	//!清空统计信息
	void Reset();

	//!在接口返回错误时，调用这个函数获取错误号
	int GetErrorNO() const { return m_iErrorNO; }

private:
	//!通过MsgID寻找到对应的TStatMsgInfo结构
	TStatMsgInfo* GetStatMsgInfo(int iMsgID);

	//!增加MsgID到统计
	int AddMsgInfo(int iMsgID);

	//!设置错误号
	void SetErrorNO(int iErrorNO) { m_iErrorNO = iErrorNO; }

private:
	CStatistic m_stStatistic;
	int m_iErrorNO; //!错误码

	short m_shMsgNum;
	TStatMsgInfo m_astMsgInfo[MAX_STAT_MSG_NUM];
};

//!一般来说只会用到一个CMsgStatistic类，所以实现一个单件方便使用
typedef Singleton<CMsgStatistic> MsgStatisticSingleton;


#endif //__MSG_STATISTIC_HPP__
///:~
