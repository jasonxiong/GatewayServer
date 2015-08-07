 

#ifndef _UDPLOG_CONFIG_H_
#define _UDPLOG_CONFIG_H_


#include "udplog_typedef.h"
#include "tinyxml.h"
#include "udplog_singleton.h"

#include <map>
#include <vector>
#include <string>

struct stHost
{
	char szIP[MAX_IPADDR_LENGTH];
	int16_t nPort;
};

class CUdpLogConfig
{
public:
	CUdpLogConfig();
	virtual ~CUdpLogConfig();

public:
	int32_t Init(const char* szFileName = DEFAULT_CONFIGFILENAME);
	
	//随机取一个host返回，返回值 < 0 表示获取失败
	int32_t GetHostRandomly(stHost& host);
	
	//根据文件名字获取对应的ID，返回值 < 0 表示获取失败
	int32_t GetLogFileID(const char* typeinfo);

    int32_t GetResourceID();
private:

	int32_t InitServerInfo(TiXmlElement* pRoot);

	int32_t InitFileTypeInfo(TiXmlElement* pRoot);

private:
	std::map<std::string, int32_t>	m_mLogFileTypeInfo;
	std::vector<stHost>				m_vHosts;
    int32_t                         m_nResourceID;
};

#define	CREATE_UDPLOGCONFIG_INSTANCE	CKingnetUdpLogSingleton<CUdpLogConfig>::CreateInstance
#define	GET_UDPLOGCONFIG_INSTANCE		CKingnetUdpLogSingleton<CUdpLogConfig>::GetInstance
#define	DESTROY_UDPLOGCONFIG_INSTANCE	CKingnetUdpLogSingleton<CUdpLogConfig>::DestroyInstance

#define _config GET_UDPLOGCONFIG_INSTANCE()

#endif
