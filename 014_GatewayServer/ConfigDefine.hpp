#ifndef __CONFIG_DEFINE_HPP__
#define __CONFIG_DEFINE_HPP__

#include "GlobalValue.hpp"

#define FILE_LOTUSCONFIG_FLASH_REQ          "../conf/FlashRequest.ini"
#define FILE_LOTUSCONFIG_FLASH_RSP          "../conf/FlashResponse.ini"

#define	FILE_LOTUSCONFIG_SERVER			    "../conf/LotusServer.tcm"
#define FILE_LOTUSCONFIG_CODEQUEUE		    "../conf/LotusCodeQueue.tcm"
#define FILE_LOTUSCONFIG_LISTENER		    "../conf/LotusListener.tcm"

#define FILE_LOTUSCONFIG_TRACE_UIN          "../conf/TraceUin.ini"
#define FILE_LOCALCONFIG_LOG                "../conf/LogSetting.tcm"

const unsigned int MAX_LISTEN_PORT_NUMBER = 16;
const unsigned int MAX_CODEQUEUE_NUMBER = 16;

#ifdef _DEBUG_
const unsigned int MAX_SERVER_ENTITY_NUMBER = 3;
#else
const unsigned int MAX_SERVER_ENTITY_NUMBER = 3;
#endif

//CodeQueue配置
typedef struct tagCodeQueueConfig
{
    int m_iCodeQueueID;

    //按位取值 0是普通输入CodeQueue 1是普通输出CodeQueue 2是默认输入CodeQueue
    unsigned int m_uiCodeQueueFlag;

    //CodeQueueKey文件路径
    char m_szKeyFileName[MAX_FILENAME_LENGTH];
    unsigned char m_ucKeyPrjID;

    //CodeQueue大小
    unsigned int m_uiShmSize;

    // CodeQueue类型，用于区分CodeQueue挂接Server的类型
    unsigned int m_uiCodeQueueType;

}TCodeQueueConfig;

typedef enum enSocketListenerType
{
	ESLT_ExternalClinet	= 1,	//外部Client
	ESLT_InternalServer	= 2,	//内部Server

}ESocketListenerType;

typedef struct tagAddress
{
	unsigned int m_uiIP;
	unsigned short m_ushPort;

}TAddress;

typedef struct tagAddressGroup
{
	unsigned int m_uiNumber;
	TAddress m_astAddress[MAX_LISTEN_PORT_NUMBER];
}TAddressGroup;


typedef enum enCodeQueueFlag
{
	ECDF_INPUT	= (unsigned int)0x00000000,	// 输入CodeQueue，初始值，不用于判断
	ECDF_OUTPUT	= (unsigned int)0x00000001,	// 输出CodeQueue
}ECodeQueueFlag;

//本字段取值用于区分CodeQueue挂接Server的类型
typedef enum enmCodeQueueType
{
	ECQT_DEFAULT = 1,	// 本CodeQueue发往默认业务服务器
	ECQT_RECORD = 3,	// 本CodeQueue发往消息录制服务器
}ECodeQueueType;

//Flash、U3D策略文件尺寸
const int POLICY_FILE_SIZE = 1024;

typedef struct tagMsgConfigBuffer
{
    short m_shLength;
    char  m_szContent[POLICY_FILE_SIZE];
}TMsgConfigBuffer;


#endif
