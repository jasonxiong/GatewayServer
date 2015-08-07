
#ifndef __LOTUS_LOG_ADAPTER_HPP__
#define __LOTUS_LOG_ADAPTER_HPP__

#include "LotusLogEngine.hpp"

#define MAX_GLOBAL_UIN_NUMBER 256

class CLotusLogAdapter
{
public:

	CLotusLogAdapter();

	virtual ~CLotusLogAdapter();

	//////////////////////////////////////////////////////////////////////////

public:

	int WriteServerLog(int iLogLevel,
		const char* szContent, ...);

	int WriteServerLog(
		const char* szContent, ...);

	int PrintServerBin(int iLogLevel,		//日志级别
		char *pcBuffer,
		int iLength);


	//////////////////////////////////////////////////////////////////////////

	int WritePlayerLog(int iLogLevel,
		unsigned int uiPlayerUin,
		const char* szContent, ...);

	int WritePlayerLog(
		unsigned int uiPlayerUin,
		const char* szContent, ...);

	int PrintPlayerBin(int iLogLevel,		//日志级别
		unsigned int uiPlayerUin,
		char *pcBuffer,
		int iLength);

	//////////////////////////////////////////////////////////////////////////

	int WriteStatLog(const char* szContent, ...);

	int WriteSafeguardStatLog(const char* szContent, ...);

    //////////////////////////////////////////////////////////////////////////

	int SetLogLevel(int iLogLevel);

	int AddLocalPlayerUinFlag(unsigned int uiPlayerUin);
	int AddGlobalPlayerUinFlag(unsigned int uiPlayerUin);
	void ClearLocalTraceFlag();
	void ClearGlobalTraceFlag();
	void InitLogPath();

    /////////////////////////////////////////////////////////////////////////
	
private:
    bool IsNeedToTracePlayer(int iLogLevel, unsigned int uiPlayerUin);
	int MakeDir(const char* szPathName);

	//////////////////////////////////////////////////////////////////////////

private:

	CLotusLogEngine m_stLogEngine;
	
	unsigned int m_auiLocalPlayerUin[MAX_GLOBAL_UIN_NUMBER];
	short m_nLocalPlayerUinCount;
	unsigned int m_auiGlobalPlayerUin[MAX_GLOBAL_UIN_NUMBER];
	short m_nGlobalPlayerUinCount;

	//文件限制
	int m_iMaxSystemLogFileSize;
	int m_iMaxSystemLogFileCount;

	int m_iMaxPlayerLogFileSize;
	int m_iMaxPlayerLogFileCount;

	int m_iMaxStatLogFileSize;
	int m_iMaxStatLogFileCount;
    char m_szLogPath[64];
};

inline bool CLotusLogAdapter::IsNeedToTracePlayer(int iLogLevel, unsigned int uiPlayerUin)
{
	//全局跟踪无需保证级别
	for(int i = 0; i < m_nGlobalPlayerUinCount; i++)
	{
		if(uiPlayerUin == m_auiGlobalPlayerUin[i])
		{
			return true;
		}
	}

	//本地跟踪需要保证级别
	if(m_stLogEngine.CheckLogLevel(iLogLevel) != 0)
	{
		return false;
	}

	for(int i = 0; i < m_nLocalPlayerUinCount; i++)
	{
		if(uiPlayerUin == m_auiLocalPlayerUin[i])
		{
			return true;
		}
	}

	return false;
}


typedef Singleton<CLotusLogAdapter> LotusServerLogEngineSingleton;

#define TRACESVR LotusServerLogEngineSingleton::instance()->WriteServerLog
#define TRACEPLAYER LotusServerLogEngineSingleton::instance()->WritePlayerLog

#define TRACEBIN LotusServerLogEngineSingleton::instance()->PrintServerBin
#define TRACEPLAYERBIN LotusServerLogEngineSingleton::instance()->PrintPlayerBin

#define TRACESTAT LotusServerLogEngineSingleton::instance()->WriteStatLog
#define TRACESAFEGUARD LotusServerLogEngineSingleton::instance()->WriteSafeguardStatLog

#define SETTRACELEVEL LotusServerLogEngineSingleton::instance()->SetLogLevel

#define ADDLOCALPLAYERUINFLAG LotusServerLogEngineSingleton::instance()->AddLocalPlayerUinFlag
#define ADDGLOBALPLAYERUINFLAG LotusServerLogEngineSingleton::instance()->AddGlobalPlayerUinFlag
#define CLEARLOCALTRACEFLAG LotusServerLogEngineSingleton::instance()->ClearLocalTraceFlag
#define CLEARGLOBALTRACEFLAG LotusServerLogEngineSingleton::instance()->ClearGlobalTraceFlag


#endif

