/**
*@file LogAdapter.hpp
*@author jasonxiong
*@date 2009-11-04
*@version 1.0
*@brief
*
*
*/

//在这添加标准库头文件
#include <string.h>

//在这添加ServerLib头文件
#include "LogAdapter.hpp"
#include "CommonDef.hpp"
#include "NowTime.hpp"
#include "UnixTime.hpp"
#include "StringUtility.hpp"

using namespace ServerLib;

CServerLogAdapter::CServerLogAdapter()
{
    TLogConfig stLogConfig;
    SAFE_STRCPY(stLogConfig.m_szPath, "../log/", sizeof(stLogConfig.m_szPath)-1);
    SAFE_STRCPY(stLogConfig.m_szBaseName, "ServerCtrl", sizeof(stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(stLogConfig.m_szExtName, ".log", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_NONE;

#ifdef _DEBUG_
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
#endif

    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
    m_iTraceDepth = 0;

    ReloadLogConfig(stLogConfig);
}

int CServerLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    int iRet = m_stLogFile.Initialize(rstLogConfig);

    if(iRet)
    {
        return -1;
    }

    return 0;
}

int CServerLogAdapter::SetLogLevel(int iLogLevel)
{
    TLogConfig stLogConfig = m_stLogFile.GetLogConfig();

    stLogConfig.m_iLogLevel = iLogLevel;

    ReloadLogConfig(stLogConfig);

    return 0;
}

int CServerLogAdapter::SetServerLogName(const char* pszLogName)
{
    if(pszLogName == NULL)
    {
        return -1;
    }

    TLogConfig stLogConfig = m_stLogFile.GetLogConfig();

    SAFE_STRCPY(stLogConfig.m_szBaseName, pszLogName, sizeof(stLogConfig.m_szBaseName)-1);

    ReloadLogConfig(stLogConfig);

    return 0;
}

int CServerLogAdapter::SetServerLogSuffix(ENMADDDATESUFFIXTYPE eType)
{
    TLogConfig stLogConfig = m_stLogFile.GetLogConfig();

    stLogConfig.m_iAddDateSuffixType = eType;

    ReloadLogConfig(stLogConfig);

    return 0;
}

CErrorLogAdapter::CErrorLogAdapter()
{
    TLogConfig stLogConfig;
    SAFE_STRCPY(stLogConfig.m_szPath, "../log/", sizeof(stLogConfig.m_szPath)-1);
    SAFE_STRCPY(stLogConfig.m_szBaseName, "Error", sizeof(stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(stLogConfig.m_szExtName, ".log", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_NONE;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FCLOSE;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    ReloadLogConfig(stLogConfig);
}

int CErrorLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    int iRet = m_stLogFile.Initialize(rstLogConfig);

    if(iRet)
    {
        return -1;
    }

    return 0;
}

CPlayerLogAdapter::CPlayerLogAdapter()
{
    SAFE_STRCPY(m_stLogConfig.m_szPath, "../log/player/", sizeof(m_stLogConfig.m_szPath)-1);
    SAFE_STRCPY(m_stLogConfig.m_szBaseName, "p", sizeof(m_stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(m_szBaseName, "p", sizeof(m_szBaseName)-1);
    SAFE_STRCPY(m_stLogConfig.m_szExtName, ".log", sizeof(m_stLogConfig.m_szExtName)-1);
    m_stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_NONE;

#ifdef _DEBUG_
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
#endif

    m_stLogConfig.m_shLogType = ELT_LOG_USE_FPRINTF;
    m_stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    m_shTraceUinNum = 0;
}

TLogConfig& CPlayerLogAdapter::GetLogConfig()
{
    return m_stLogConfig;
}

int CPlayerLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    m_stLogConfig = rstLogConfig;
    SAFE_STRCPY(m_szBaseName, rstLogConfig.m_szBaseName, sizeof(m_szBaseName)-1);

    return 0;
}

int CPlayerLogAdapter::ClearTraceUinSet()
{
    m_shTraceUinNum = 0;

    return 0;
}

int CPlayerLogAdapter::AddTraceUin(unsigned int uiUin)
{
    if(m_shTraceUinNum >= MAX_TRACE_UIN_NUMBER)
    {
        return -1;
    }

    m_auiTraceUin[m_shTraceUinNum++]  = uiUin;

    return 0;
}

int CPlayerLogAdapter::CheckUin(unsigned int uiUin)
{
    //在全部日志都打的情况下不判断Uin是否在白名单中
    if(m_stLogConfig.m_iLogLevel == (int)LOG_LEVEL_ANY)
    {
        return 0;
    }

    short i;
    for(i = 0; i < m_shTraceUinNum; ++i)
    {
        if(m_auiTraceUin[i] == uiUin)
        {
            return 0;
        }
    }

    return -1;
}

int CPlayerLogAdapter::WriteLog(unsigned int uiUin, const char* pszFile,
                                int iLine, const char* pszFunc, const char *pcContent, ...)
{
    if(CheckUin(uiUin))
    {
        return -1;
    }

    SAFE_SPRINTF(m_stLogConfig.m_szBaseName, sizeof(m_stLogConfig.m_szBaseName)-1,
                 "%s%u", m_szBaseName, uiUin);
    m_stLogConfig.m_szBaseName[MAX_FILENAME_LENGTH-1] = '\0';

    m_stLogFile.Initialize(m_stLogConfig);

    va_list ap;
    va_start(ap, pcContent);
    int iRet = m_stLogFile.WriteLogExVA(LOG_LEVEL_ANY, -1, pszFile, iLine, pszFunc, pcContent, ap);
    va_end(ap);

    return iRet;
}

FILE* CPlayerLogAdapter::GetPlayerFile(unsigned int uiUin)
{
    if(CheckUin(uiUin))
    {
        return NULL;
    }

    SAFE_SPRINTF(m_stLogConfig.m_szBaseName, sizeof(m_stLogConfig.m_szBaseName)-1,
                 "%s%u", m_szBaseName, uiUin);
    m_stLogConfig.m_szBaseName[MAX_FILENAME_LENGTH-1] = '\0';

    m_stProtoLogFile.Initialize(m_stLogConfig);
    m_stProtoLogFile.OpenLogFile();

    return m_stProtoLogFile.GetFilePointer();
}

FILE* CPlayerLogAdapter::GetPlayerProtoFile(unsigned int uiUin)
{
    if(CheckUin(uiUin))
    {
        return NULL;
    }

    SAFE_SPRINTF(m_stLogConfig.m_szBaseName, sizeof(m_stLogConfig.m_szBaseName)-1,
                 "%s%u_proto", m_szBaseName, uiUin);
    m_stLogConfig.m_szBaseName[MAX_FILENAME_LENGTH-1] = '\0';

    m_stProtoLogFile.Initialize(m_stLogConfig);
    m_stProtoLogFile.OpenLogFile();

    return m_stProtoLogFile.GetFilePointer();
}

CMsgLogAdapter::CMsgLogAdapter()
{
    TLogConfig stLogConfig;
    SAFE_STRCPY(stLogConfig.m_szPath, "../log/", sizeof(stLogConfig.m_szPath)-1);
    SAFE_STRCPY(stLogConfig.m_szBaseName, "msg", sizeof(stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(stLogConfig.m_szExtName, ".log", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_NONE;

#ifdef _DEBUG_
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
#endif

    stLogConfig.m_shLogType = ELT_LOG_USE_FPRINTF;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    m_shTraceMsgNum = 0;

    ReloadLogConfig(stLogConfig);
}

int CMsgLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    int iRet = m_stLogFile.Initialize(rstLogConfig);
    if(iRet)
    {
        return -1;
    }

    return 0;
}

int CMsgLogAdapter::ClearTraceMsgSet()
{
    m_shTraceMsgNum = 0;

    return 0;
}

int CMsgLogAdapter::AddTraceMsg(int iMsgID)
{
    if(m_shTraceMsgNum >= MAX_TRACE_MSG_NUMBER)
    {
        return -1;
    }

    m_aiTraceMsg[m_shTraceMsgNum++]  = iMsgID;

    return 0;
}

int CMsgLogAdapter::CheckMsg(int iMsgID)
{
    short i;
    for(i = 0; i < m_shTraceMsgNum; ++i)
    {
        if(m_aiTraceMsg[i] == iMsgID)
        {
            return 0;
        }
    }

    return -1;
}

int CMsgLogAdapter::WriteLog(int iLogLevel, int iMsgID,
                             const char* pszFile, int iLine,
                             const char* pszFunc, const char *pcContent, ...)
{
    if(CheckMsg(iMsgID))
    {
        return -1;
    }

    va_list ap;
    va_start(ap, pcContent);
    int iRet = m_stLogFile.WriteLogExVA(iLogLevel, -1, pszFile, iLine, pszFunc, pcContent, ap);
    va_end(ap);

    return iRet;
}

int CMsgLogAdapter::WriteLog(int iLogLevel, const char* pszFile, int iLine,
                             const char* pszFunc, const char *pcContent, ...)
{
    va_list ap;
    va_start(ap, pcContent);
    int iRet = m_stLogFile.WriteLogExVA(iLogLevel, -1, pszFile, iLine, pszFunc, pcContent, ap);
    va_end(ap);

    return iRet;
}


CBillLogAdapter::CBillLogAdapter()
{
    TLogConfig stLogConfig;
    time_t tNow = NowTimeSingleton::Instance()->GetNowTime();
    CUnixTime stUnixTime(tNow);
    m_iCurMonth = stUnixTime.GetMonth();
    SAFE_SPRINTF(stLogConfig.m_szPath, sizeof(stLogConfig.m_szPath)-1,
                 "../bills/%04d-%02d/", stUnixTime.GetYear(), m_iCurMonth);
    SAFE_STRCPY(stLogConfig.m_szBaseName, "bill", sizeof(stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(stLogConfig.m_szExtName, ".log", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    ReloadLogConfig(stLogConfig);
}

int CBillLogAdapter::CheckBillPath()
{
    time_t tNow = NowTimeSingleton::Instance()->GetNowTime();
    CUnixTime stUnixTime(tNow);

    int iMonth = stUnixTime.GetMonth();

    if(iMonth != m_iCurMonth)
    {
        TLogConfig stLogConfig = m_stLogFile.GetLogConfig();
        SAFE_SPRINTF(stLogConfig.m_szPath, sizeof(stLogConfig.m_szPath)-1,
                     "../bills/%04d-%02d/", stUnixTime.GetYear(), iMonth);
        ReloadLogConfig(stLogConfig);
        m_iCurMonth = iMonth;
    }

    return 0;
}

int CBillLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    int iRet = m_stLogFile.Initialize(rstLogConfig);

    if(iRet)
    {
        return -1;
    }

    return 0;
}

int CBillLogAdapter::SetBillLogName(const char* pszLogName)
{
    if(pszLogName == NULL)
    {
        return -1;
    }

    TLogConfig stLogConfig = m_stLogFile.GetLogConfig();

    SAFE_STRCPY(stLogConfig.m_szBaseName, pszLogName, sizeof(stLogConfig.m_szBaseName)-1);

    ReloadLogConfig(stLogConfig);

    return 0;
}


CThreadLogAdapter::CThreadLogAdapter()
{
    SAFE_STRCPY(m_stLogConfig.m_szPath, "../log/thread/", sizeof(m_stLogConfig.m_szPath)-1);
    SAFE_STRCPY(m_stLogConfig.m_szBaseName, "thread", sizeof(m_stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(m_szBaseName, "thread", sizeof(m_szBaseName)-1);
    SAFE_STRCPY(m_stLogConfig.m_szExtName, ".log", sizeof(m_stLogConfig.m_szExtName)-1);
    m_stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_NONE;

#ifdef _DEBUG_
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
#endif

    m_stLogConfig.m_shLogType = ELT_LOG_USE_FPRINTF;
    m_stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
}

int CThreadLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    m_stLogConfig = rstLogConfig;
    SAFE_STRCPY(m_szBaseName, rstLogConfig.m_szBaseName, sizeof(m_szBaseName)-1);

    return 0;
}

int CThreadLogAdapter::WriteLog(int iThreadID, const char* pszFile,
                                int iLine, const char* pszFunc, const char *pcContent, ...)
{
    SAFE_SPRINTF(m_stLogConfig.m_szBaseName, sizeof(m_stLogConfig.m_szBaseName)-1,
                 "%s%u", m_szBaseName, iThreadID);
    m_stLogConfig.m_szBaseName[MAX_FILENAME_LENGTH-1] = '\0';

    m_stLogFile.Initialize(m_stLogConfig);

    va_list ap;
    va_start(ap, pcContent);
    int iRet = m_stLogFile.WriteLogExVA(LOG_LEVEL_ANY, -1, pszFile, iLine, pszFunc, pcContent, ap);
    va_end(ap);

    return iRet;
}


CRoomLogAdapter::CRoomLogAdapter()
{
    SAFE_STRCPY(m_stLogConfig.m_szPath, "../log/room/", sizeof(m_stLogConfig.m_szPath)-1);
    SAFE_STRCPY(m_stLogConfig.m_szBaseName, "room", sizeof(m_stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(m_szBaseName, "room", sizeof(m_szBaseName)-1);
    SAFE_STRCPY(m_stLogConfig.m_szExtName, ".log", sizeof(m_stLogConfig.m_szExtName)-1);
    m_stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_NONE;

#ifdef _DEBUG_
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
#endif

    m_stLogConfig.m_shLogType = ELT_LOG_USE_FPRINTF;
    m_stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
}

int CRoomLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    m_stLogConfig = rstLogConfig;
    SAFE_STRCPY(m_szBaseName, rstLogConfig.m_szBaseName, sizeof(m_szBaseName)-1);

    return 0;
}

int CRoomLogAdapter::WriteLog(int iRoomID, const char* pszFile,
                              int iLine, const char* pszFunc, const char *pcContent, ...)
{
    SAFE_SPRINTF(m_stLogConfig.m_szBaseName, sizeof(m_stLogConfig.m_szBaseName)-1,
                 "%s%u", m_szBaseName, iRoomID);
    m_stLogConfig.m_szBaseName[MAX_FILENAME_LENGTH-1] = '\0';

    m_stLogFile.Initialize(m_stLogConfig);

    va_list ap;
    va_start(ap, pcContent);
    int iRet = m_stLogFile.WriteLogExVA(LOG_LEVEL_ANY, -1, pszFile, iLine, pszFunc, pcContent, ap);
    va_end(ap);

    return iRet;
}


CClientLogAdapter::CClientLogAdapter()
{
    SAFE_STRCPY(m_stLogConfig.m_szPath, "../log/client/", sizeof(m_stLogConfig.m_szPath)-1);
    SAFE_STRCPY(m_stLogConfig.m_szBaseName, "client", sizeof(m_stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(m_szBaseName, "client", sizeof(m_szBaseName)-1);
    SAFE_STRCPY(m_stLogConfig.m_szExtName, ".log", sizeof(m_stLogConfig.m_szExtName)-1);
    m_stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_NONE;

#ifdef _DEBUG_
    m_stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
#endif

    m_stLogConfig.m_shLogType = ELT_LOG_USE_FPRINTF;
    m_stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
}

int CClientLogAdapter::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    m_stLogConfig = rstLogConfig;
    SAFE_STRCPY(m_szBaseName, rstLogConfig.m_szBaseName, sizeof(m_szBaseName)-1);

    return 0;
}

int CClientLogAdapter::WriteLog(int iUin, const char* pszFile,
                                int iLine, const char* pszFunc, const char *pcContent, ...)
{
    SAFE_SPRINTF(m_stLogConfig.m_szBaseName, sizeof(m_stLogConfig.m_szBaseName)-1,
                 "%s%u", m_szBaseName, iUin);
    m_stLogConfig.m_szBaseName[MAX_FILENAME_LENGTH-1] = '\0';

    m_stLogFile.Initialize(m_stLogConfig);

    va_list ap;
    va_start(ap, pcContent);
    int iRet = m_stLogFile.WriteLogExVA(LOG_LEVEL_ANY, -1, pszFile, iLine, pszFunc, pcContent, ap);
    va_end(ap);

    return iRet;
}



