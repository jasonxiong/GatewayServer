
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "TimeTools.hpp"

#include "LotusLogEngine.hpp"

CLotusLogEngine::CLotusLogEngine()
{
	m_uiLogLevel = 0;
}

CLotusLogEngine::~CLotusLogEngine()
{

}

int CLotusLogEngine::InitLog(
				char *pcFileName,
				int iMaxFileSize, int iMaxFileCount)
{
	strncpy(m_szFileName, pcFileName, sizeof(m_szFileName));
	m_iMaxFileSize = iMaxFileSize;
	m_iMaxFileCount = iMaxFileCount;
	m_iSimpleTrace = 1;
	return 0;
}

int CLotusLogEngine::WriteLog(
			 char *pcErrInfo,
			 const char *pcContent, ...)
{
	FILE* pFile;
	char szCurDateTime[MAX_DATETIME_LENGTH];
	pFile = fopen(m_szFileName, "a+");
	if(!pFile)
	{
		if(pcErrInfo)
		{
			strcpy(pcErrInfo, "Fail to open log file");
		}
		return -1;
	}

	GetCurDateTimeString((char *)szCurDateTime);

	fprintf(pFile, "[%s] ", szCurDateTime);

	va_list ap;
	va_start(ap, pcContent);
	vfprintf(pFile, pcContent, ap);
	va_end(ap);

	fclose(pFile);

	return ShiftFile(m_szFileName, m_iMaxFileSize, m_iMaxFileCount, pcErrInfo);
}

int CLotusLogEngine::WriteLog(char *pcFileName,
			 int iMaxFileSize, int iMaxFileCount,
			 char *pcErrInfo,
			 const char *pcContent, va_list ap)
{
	FILE* pFile;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDateTime[MAX_DATETIME_LENGTH];

	sprintf(szFileName, "%s.log", pcFileName);
	pFile = fopen(szFileName, "a+");
	if(!pFile)
	{
		if(pcErrInfo)
		{
			strcpy(pcErrInfo, "Fail to open log file");
		}
		return -1;
	}

	GetCurDateTimeString((char *)szCurDateTime);

	fprintf(pFile, "[%s] ", szCurDateTime);

	//va_list ap;
	//va_start(ap, pcContent);
	vfprintf(pFile, pcContent, ap);
	//va_end(ap);

	fclose(pFile);

	return ShiftFile(pcFileName, iMaxFileSize, iMaxFileCount, pcErrInfo);
}

int CLotusLogEngine::WriteLog(char *pcFileName,
			 char *pcErrInfo,
			 const char *pcContent, va_list ap)
{
	FILE* pFile;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDateTime[MAX_DATETIME_LENGTH];

	sprintf(szFileName, "%s.log", pcFileName);
	pFile = fopen(szFileName, "a+");
	if(!pFile)
	{
		if(pcErrInfo)
		{
			strcpy(pcErrInfo, "Fail to open log file");
		}
		return -1;
	}

	GetCurDateTimeString((char *)szCurDateTime);

	fprintf(pFile, "[%s] ", szCurDateTime);

	//va_list ap;
	//va_start(ap, pcContent);
	vfprintf(pFile, pcContent, ap);
	//va_end(ap);

	fclose(pFile);

	return 0;
}



int CLotusLogEngine::PrintBin(char *pcFileName,
						 char *pcBuffer,
						 int iLength,
						 char *pcErrInfo)
{
	if(iLength <= 0 || pcBuffer == NULL)
	{
		return -1;
	}

	int i;

	FILE* pFile;
	char szFileName[MAX_FILENAME_LENGTH];
	char szCurDateTime[MAX_DATETIME_LENGTH];

	sprintf(szFileName, "%s.log", pcFileName);
	pFile = fopen(szFileName, "a+");
	if(!pFile)
	{
		if(pcErrInfo)
		{
			strcpy(pcErrInfo, "Fail to open log file");
		}
		return -1;
	}

	GetCurDateTimeString((char *)szCurDateTime);

	fprintf(pFile, "[%s] Buffer Length = %d", szCurDateTime, iLength);
	//TBD
	if(iLength > 1024)
	{
		iLength = 1024;
	}
	for(i = 0; i < iLength; i++)
	{
		if(!(i % 16))
		{
			fprintf(pFile, "\n[%s] %04d>\t", szCurDateTime, i / 16 + 1);
		}
		fprintf(pFile, "%02X ", (unsigned char)pcBuffer[i]);
	}
	fprintf(pFile, "\n");

	fclose(pFile);



	return 0;
}

int CLotusLogEngine::ShiftFile(char *pcFileName,
					 int iMaxFileSize, int iMaxFileCount,
					 char *pcErrInfo)
{
	struct stat stStat;
	char szFileName[MAX_FILENAME_LENGTH];
	char szAnotherFileName[MAX_FILENAME_LENGTH];
	int i;

	sprintf(szFileName, "%s.log", pcFileName);

	if(stat(szFileName, &stStat) < 0)
	{
		if(pcErrInfo)
		{
			strcpy(pcErrInfo, "Fail to get file status");
		}
		return -1;
	}

	if(stStat.st_size < iMaxFileSize)
	{
		return 0;
	}

	sprintf(szFileName, "%s_%d.log", pcFileName, iMaxFileCount - 1);
	if(access(szFileName, F_OK) == 0)
	{
		if(remove(szFileName) < 0)
		{
			if(pcErrInfo != NULL)
			{
				strcpy(pcErrInfo, "Fail to remove oldest log file");
			}
			return -1;
		}
	}

	for(i = iMaxFileCount - 2; i >= 0; i--)
	{
		if(i == 0)
		{
			sprintf(szFileName,"%s.log", pcFileName);
		}
		else
		{
			sprintf(szFileName,"%s_%d.log", pcFileName, i);
		}

		if(access(szFileName, F_OK) == 0)
		{
			sprintf(szAnotherFileName, "%s_%d.log", pcFileName, i + 1);
			if(rename(szFileName, szAnotherFileName) < 0)
			{
				if(pcErrInfo)
				{
					strcpy(pcErrInfo, "Fail to remove oldest log file");
				}
				return -1;
			}
		}
	}
	return 0;
}

int CLotusLogEngine::SetLogLevel(unsigned int uiLogLevel)
{
	m_uiLogLevel = uiLogLevel;
	return 0;
}

int CLotusLogEngine::CheckLogLevel(unsigned int uiLogLevel)
{
	if((m_uiLogLevel & uiLogLevel) || (LOG_LEVEL_ANY == uiLogLevel))
	{
		return 0;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
