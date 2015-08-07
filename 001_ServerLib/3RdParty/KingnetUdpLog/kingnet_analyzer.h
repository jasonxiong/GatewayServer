#ifndef _KINGNET_ANALYZER_H_
#define _KINGNET_ANALYZER_H_

#include "udplog_typedef.h"
#include "udplog_singleton.h"

#include <map>
#include <string>
#include <cstdio>
#include <ctime>
#include <string.h>
#include <stdlib.h>

class CUserLogger
{
public:
	int32_t m_nUid;
	int32_t m_nErrorNo;
	char	m_szUserIP[MAX_IPADDR_LENGTH];
	int32_t m_nTime;
	char	m_szUserMsg[MAX_USERMSG_LENGTH];
	bool	m_bForbidden;
	int32_t	m_nSendCount;
	char m_szLogBuffer[MAX_LOG_LENGTH];

public:
	CUserLogger(int32_t uid)
	{
		m_nUid		= uid;
		m_nErrorNo	= 0;
		m_bForbidden= false;
		m_szUserIP[0]='\0';
		m_nTime		= (int32_t)time(NULL);
		m_nSendCount= 0;
		m_szLogBuffer[0] = '\0';

	}

	void SetUserIP(const char* ip)
	{
#ifndef WIN32
		strncpy(m_szUserIP, ip, MAX_IPADDR_LENGTH-1);
#else
		strncpy_s(m_szUserIP, sizeof(m_szUserIP), ip, strlen(ip));
#endif
	}

	void SetUserMsg(const char* msg)
	{
#ifndef WIN32
		strncpy(m_szUserMsg, msg, MAX_USERMSG_LENGTH-1);
#else
		strncpy_s(m_szUserMsg, sizeof(m_szUserMsg), msg, strlen(msg));
#endif

	}
	//send msg to remote
	int32_t LogMsg(const char* str1, const char* str2, const char* str3, const char* str4, const char* str5, const char* str6, const char*type = "login",int32_t nCount = 1);

private:

	int32_t SendUDPLog(int32_t uid, int32_t type, const char* msg);
};

class CKingnetAnalyzer
{
private:
	std::map<std::string, CUserLogger*> m_mUserInfo;


public:
	CKingnetAnalyzer()
	{
	}
	int32_t Init(const char* path = DEFAULT_CONFIGFILENAME);

public:
	CUserLogger* GetInstance(const char* uid)
	{
		if(NULL == uid)
		{
			return NULL;
		}
		std::map<std::string, CUserLogger*>::iterator it = m_mUserInfo.find(std::string(uid));
		if(it == m_mUserInfo.end())
		{
			//not found, new one
			int32_t iUid = atoi(uid);
			if(0 == iUid)
			{
				iUid = 1;
			}
			CUserLogger *info = new CUserLogger(iUid);
			m_mUserInfo[std::string(uid)] = info;
		}
		return m_mUserInfo[uid];
	}
	void DestoryInstance(const char* uid)
	{
		std::map<std::string, CUserLogger*>::iterator it = m_mUserInfo.find(std::string(uid));
		if(it != m_mUserInfo.end())
		{
			CUserLogger *info = m_mUserInfo[std::string(uid)];
			delete info;
			m_mUserInfo.erase(uid);
		}
	}
};
#define	CREATE_ANALYZER_INSTANCE	CKingnetUdpLogSingleton<CKingnetAnalyzer>::CreateInstance
#define	GET_ANALYZER_INSTANCE		CKingnetUdpLogSingleton<CKingnetAnalyzer>::GetInstance
#define	DESTROY_ANALYZER_INSTANCE	CKingnetUdpLogSingleton<CKingnetAnalyzer>::DestroyInstance

#define _analyzer GET_ANALYZER_INSTANCE()
#endif
