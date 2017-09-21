
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include "TimeTools.hpp"
#include "LotusLogAdapter.hpp"
#include "ConfigDefine.hpp"
#include "base.hpp"

CLotusLogAdapter::CLotusLogAdapter()
{
	ClearLocalTraceFlag();
	ClearGlobalTraceFlag();
	InitLogPath();

	m_iMaxSystemLogFileSize = 0x4000000;
	m_iMaxSystemLogFileCount = 9;

	m_iMaxPlayerLogFileSize = 0x1000000;
	m_iMaxPlayerLogFileCount = 1;

	m_iMaxStatLogFileSize = 0x1000000;
	m_iMaxStatLogFileCount = 1;
}

CLotusLogAdapter::~CLotusLogAdapter()
{

}

//////////////////////////////////////////////////////////////////////////

int CLotusLogAdapter::WriteServerLog(int iLogLevel, const char* szContent, ...)
{
	if(m_stLogEngine.CheckLogLevel(iLogLevel) != 0)
	{
		return -1;
	}

	int iRet = -1;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/LotusServer_%s", m_szLogPath, szCurDate);

	va_list ap;

	va_start(ap, szContent);
	iRet = m_stLogEngine.WriteLog(szFileName,
		m_iMaxSystemLogFileSize, m_iMaxSystemLogFileCount,
		szErrInfo,
		szContent,
		ap);
	va_end(ap);

	return iRet;
}

int CLotusLogAdapter::WriteServerLog(const char* szContent, ...)
{
	int iRet = -1;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/LotusServer_%s", m_szLogPath, szCurDate);

	va_list ap;

	va_start(ap, szContent);
	iRet = m_stLogEngine.WriteLog(szFileName,
		m_iMaxSystemLogFileSize, m_iMaxSystemLogFileCount,
		szErrInfo,
		szContent,
		ap);
	va_end(ap);

	return iRet;
}

int CLotusLogAdapter::PrintServerBin(int iLogLevel,
									 char *pcBuffer, int iLength)
{
	if(m_stLogEngine.CheckLogLevel(iLogLevel) != 0)
	{
		return -1;
	}

	int iRet = -1;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/LotusServer_%s", m_szLogPath, szCurDate);

	iRet = m_stLogEngine.PrintBin(szFileName,
		pcBuffer,
		iLength,
		szErrInfo);

	return iRet;
}


//////////////////////////////////////////////////////////////////////////

int CLotusLogAdapter::WritePlayerLog(int iLogLevel,
									   unsigned int uiPlayerUin,
									   const char* szContent, ...)
{
	if(m_stLogEngine.CheckLogLevel(iLogLevel) != 0)
	{
		return -1;
	}

	int iRet = -1;
	char szPathName[MAX_FILENAME_LENGTH];
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	if(!IsNeedToTracePlayer(iLogLevel, uiPlayerUin))
	{
		return -1;
	}

	sprintf(szPathName, "%s/player", m_szLogPath);

	if(MakeDir(szPathName) < 0)
	{
		return -1;
	}

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/p%u_%s", szPathName, uiPlayerUin, szCurDate);

	va_list ap;

	va_start(ap, szContent);
	iRet = m_stLogEngine.WriteLog(szFileName,
		m_iMaxPlayerLogFileSize, m_iMaxPlayerLogFileCount,
		szErrInfo,
		szContent,
		ap);
	va_end(ap);

	return iRet;
}

int CLotusLogAdapter::WritePlayerLog(unsigned int uiPlayerUin,
									   const char* szContent, ...)
{	
	int iRet = -1;
	char szPathName[MAX_FILENAME_LENGTH];
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	if(!IsNeedToTracePlayer(LOG_LEVEL_ANY, uiPlayerUin))
	{
		return -1;
	}

	sprintf(szPathName, "%s/player", m_szLogPath);

	if(MakeDir(szPathName) < 0)
	{
		return -1;
	}

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/p%u_%s", szPathName, uiPlayerUin, szCurDate);

	va_list ap;

	va_start(ap, szContent);
	iRet = m_stLogEngine.WriteLog(szFileName,
		m_iMaxPlayerLogFileSize, m_iMaxPlayerLogFileCount,
		szErrInfo,
		szContent,
		ap);
	va_end(ap);

	return iRet;
}


int CLotusLogAdapter::PrintPlayerBin(int iLogLevel,
									   unsigned int uiPlayerUin,
									   char *pcBuffer,
									   int iLength)
{
	if(m_stLogEngine.CheckLogLevel(iLogLevel) != 0)
	{
		return -1;
	}

	int iRet = -1;
	char szPathName[MAX_FILENAME_LENGTH];
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	if(!IsNeedToTracePlayer(iLogLevel, uiPlayerUin))
	{
		return -1;
	}

	sprintf(szPathName, "%s/player", m_szLogPath);

	if(MakeDir(szPathName) < 0)
	{
		return -1;
	}

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/p%u_%s", szPathName, uiPlayerUin, szCurDate);

	iRet = m_stLogEngine.PrintBin(szFileName,
		pcBuffer,
		iLength,
		szErrInfo);

	return iRet;
}

//////////////////////////////////////////////////////////////////////////

int CLotusLogAdapter::WriteSafeguardStatLog(const char* szContent, ...)
{
	int iRet = -1;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/LotusSafeguradStat_%s",m_szLogPath, szCurDate);

	va_list ap;

	va_start(ap, szContent);
	iRet = m_stLogEngine.WriteLog(szFileName,
		m_iMaxStatLogFileSize, m_iMaxStatLogFileCount,
		szErrInfo,
		szContent,
		ap);
	va_end(ap);

	return iRet;
}

int CLotusLogAdapter::WriteStatLog(const char* szContent, ...)
{
	int iRet = -1;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDate[MAX_DATE_LENGTH];
	char szErrInfo[256];

	GetDateString((char *)szCurDate);
	sprintf(szFileName, "%s/LotusStat_%s", m_szLogPath, szCurDate);

	va_list ap;

	va_start(ap, szContent);
	iRet = m_stLogEngine.WriteLog(szFileName,
		m_iMaxStatLogFileSize, m_iMaxStatLogFileCount,
		szErrInfo,
		szContent,
		ap);
	va_end(ap);

	return iRet;
}

//////////////////////////////////////////////////////////////////////////

int CLotusLogAdapter::SetLogLevel(int iLogLevel)
{
	return m_stLogEngine.SetLogLevel(iLogLevel);
}

int CLotusLogAdapter::AddLocalPlayerUinFlag(unsigned int uiPlayerUin)
{
	if(m_nLocalPlayerUinCount >= 256)	//
	{
		return -1;
	}
	if(m_nLocalPlayerUinCount < 0)
	{
		m_nLocalPlayerUinCount = 0;
	}
	m_auiLocalPlayerUin[m_nLocalPlayerUinCount] = uiPlayerUin;
	m_nLocalPlayerUinCount++;
	return 0;
}

int CLotusLogAdapter::AddGlobalPlayerUinFlag(unsigned int uiPlayerUin)
{
	if(m_nGlobalPlayerUinCount >= MAX_GLOBAL_UIN_NUMBER)	//
	{
		return -1;
	}
	if(m_nGlobalPlayerUinCount < 0)
	{
		m_nGlobalPlayerUinCount = 0;
	}
	m_auiGlobalPlayerUin[m_nGlobalPlayerUinCount] = uiPlayerUin;
	m_nGlobalPlayerUinCount++;
	return 0;
}

void CLotusLogAdapter::ClearLocalTraceFlag()
{
	m_nLocalPlayerUinCount = 0;
}

void CLotusLogAdapter::ClearGlobalTraceFlag()
{
	m_nGlobalPlayerUinCount = 0;
}

//////////////////////////////////////////////////////////////////////////

int CLotusLogAdapter::MakeDir(const char* szPathName)
{
	DIR* dpLog = opendir((const char *)szPathName);
	if(!dpLog)
	{
		if(mkdir(szPathName, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			return -1;
		}
	}
	else
	{
		closedir(dpLog);
	}

	return 0;
}


void CLotusLogAdapter::InitLogPath()
{
	FILE* fpServerConfig = NULL;
	char szLine[512] = {0};
	char szConfig[64] = {0};
	char szSign[5] = {0};
	char szValue[64] = {0};

	strncpy(m_szLogPath,"../log",sizeof(m_szLogPath));

	fpServerConfig = fopen(FILE_LOCALCONFIG_LOG, "r");
	if(!fpServerConfig)
	{
		printf("Open File %s Failed.\n", FILE_LOCALCONFIG_LOG);
		return ;
	}

	while(!feof(fpServerConfig))
	{
		fgets((char*)szLine, sizeof(szLine), fpServerConfig);
		TrimStr((char*)szLine);

		if (szLine[0] == '#')
		{
			continue;
		}

		int iReadColomns = sscanf(szLine, "%s%s%s", szConfig, szSign, szValue);
		if (iReadColomns != 3)
		{
			continue;
		}

		if(!strncmp(szConfig,"LogPath",sizeof(szConfig)))
		{
			strncpy(m_szLogPath, szValue, sizeof(m_szLogPath));
		}

		if(!strncmp(szConfig,"LogLevel",sizeof(szConfig)))
		{
			SetLogLevel(atoi(szValue));
		}
	}

	MakeDir(m_szLogPath);

	fclose(fpServerConfig);
}

