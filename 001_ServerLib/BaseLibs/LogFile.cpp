/**
*@file LogFile.cpp
*@author jasonxiong
*@date 2009.08.20
*@version 1.0
*@brief CLogFile的实现文件
*
*
*/

//在这添加标准库头文件
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <dirent.h>

//在这添加ServerLib头文件
#include "TimeUtility.hpp"
#include "NowTime.hpp"
#include "LogFile.hpp"
#include "FileUtility.hpp"
#include "ErrorDef.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

CLogFile::CLogFile()
{
    m_pFile = NULL;
    memset(&m_stLogConfig, 0, sizeof(m_stLogConfig));
    m_szCurLogName[0] = '\0';

    m_iLogCount = 0;
    m_iLastLogHour  = 0;
    m_tLastChkFileTime = time(NULL);
    m_tLastCleanupFileTime = time(NULL);

    m_bEnableFLF = false;
    m_iErrorNO = 0;
}

CLogFile::~CLogFile()
{
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
}

int CLogFile::Initialize(const char* pszLogPath, const char* pszLogName, int iLogLevel)
{
    if(pszLogPath == NULL || pszLogName == NULL)
    {
        SetErrorNO(EEN_LOGFILE__NULL_POINTER);

        return -1;
    }

    memset(&m_stLogConfig, 0, sizeof(m_stLogConfig));
    SAFE_STRCPY(m_stLogConfig.m_szPath, pszLogPath, sizeof(m_stLogConfig.m_szPath)-1);
    SAFE_STRCPY(m_stLogConfig.m_szBaseName, pszLogName, sizeof(m_stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(m_stLogConfig.m_szExtName, ".log", sizeof(m_stLogConfig.m_szExtName)-1);
    m_stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    m_stLogConfig.m_iLogLevel = iLogLevel;
    m_stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    m_stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
    m_stLogConfig.m_iChkFileInterval = DEFAULT_CHECK_FILE_INTERVAL;

    m_szCurLogName[0] = '\0';
    m_iLogCount = 0;

    CheckPath();

    return 0;
}

int CLogFile::Initialize(const TLogConfig& rstLogConfig)
{
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    m_stLogConfig = rstLogConfig;

    if(m_stLogConfig.m_iChkFileInterval <= 0)
    {
        m_stLogConfig.m_iChkFileInterval = DEFAULT_CHECK_FILE_INTERVAL;
    }

    if(m_stLogConfig.m_iChkSizeSteps <= 0)
    {
        m_stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    }

    if(m_stLogConfig.m_iMaxFileSize <= 0)
    {
        m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    }

    if(m_stLogConfig.m_iMaxFileCount <= 0)
    {
        m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
    }

    m_szCurLogName[0] = '\0';
    m_iLogCount = 0;

    CheckPath();

    return 0;
}

int CLogFile::CheckWriteLogLevel(int iLogLevel)
{
    if(iLogLevel <= 0 || BIT_ENABLED(m_stLogConfig.m_iLogLevel, iLogLevel))
    {
        return 0;
    }

    return -1;
}

int CLogFile::CheckPath()
{
    char* pszLogPath = m_stLogConfig.m_szPath;
    pszLogPath[MAX_FILENAME_LENGTH-1] = '\0'; //作个保护

    if(pszLogPath[0] != '\0')
    {
        int iPathLenth = strlen(pszLogPath);
        if(pszLogPath[iPathLenth-1] != '/')
        {
            strncat(pszLogPath, "/", MAX_FILENAME_LENGTH-1);
        }

        char szDirPath[MAX_FILENAME_LENGTH];

        int i;
        for(i = 0; i < iPathLenth; ++i)
        {
            if(pszLogPath[i] == '/')
            {
                SAFE_STRCPY(szDirPath, pszLogPath, i+1);
                szDirPath[i+1] = '\0';
                CFileUtility::MakeDir(szDirPath);
            }
        }
    }

    return 0;
}

int CLogFile::GetRealLogName(int iLogIndex, char* szRealName, int iNameSize)
{
    if(szRealName == NULL || iNameSize < 0)
    {
        SetErrorNO(EEN_LOGFILE__NULL_POINTER);

        return -1;
    }

    char szCurDate[MAX_DATE_LENGTH];
    time_t tNow = time(NULL);
    CTimeUtility::ConvertUnixTimeToDateString(tNow, szCurDate);

    switch(m_stLogConfig.m_iAddDateSuffixType)
    {
    case EADST_DATE_YMD:
    {
        if(iLogIndex > 0)
        {
            SAFE_SPRINTF(szRealName, iNameSize-1,
                         "%s%s_%s#%d%s", m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName,
                         szCurDate, iLogIndex, m_stLogConfig.m_szExtName);
        }
        else
        {
            SAFE_SPRINTF(szRealName, iNameSize-1,
                         "%s%s_%s%s", m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName,
                         szCurDate, m_stLogConfig.m_szExtName);
        }

        break;
    }

    case EADST_DATE_YMDH:
    {
        int iHour = 0;
        time_t tNow = time(NULL);
        struct tm stTempTm;
        struct tm *pTempTm = localtime_r(&tNow, &stTempTm);
        iHour = pTempTm->tm_hour;

        if(iLogIndex > 0)
        {
            SAFE_SPRINTF(szRealName, iNameSize-1,
                         "%s%s_%s-%d#%d%s", m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName,
                         szCurDate, iHour, iLogIndex, m_stLogConfig.m_szExtName);
        }
        else
        {
            SAFE_SPRINTF(szRealName, iNameSize-1,
                         "%s%s_%s-%d%s", m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName,
                         szCurDate, iHour, m_stLogConfig.m_szExtName);
        }

        break;
    }

    default:
    {
        if(iLogIndex > 0)
        {
            SAFE_SPRINTF(szRealName, iNameSize-1,
                         "%s%s#%d%s", m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName,
                         iLogIndex, m_stLogConfig.m_szExtName);
        }
        else
        {
            SAFE_SPRINTF(szRealName, iNameSize-1,
                         "%s%s%s", m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName,
                         m_stLogConfig.m_szExtName);
        }

        break;
    }
    }

    return 0;
}

int CLogFile::OpenLogFile()
{
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    m_szCurLogName[0] = '\0';
    GetRealLogName(0, m_szCurLogName, MAX_FILENAME_LENGTH);

    m_pFile = fopen(m_szCurLogName, "a+");

    if(!m_pFile)
    {
        SetErrorNO(EEN_LOGFILE__OPEN_FILE_FAILED);

        return -1;
    }

    return 0;
}

int CLogFile::BackupLog()
{
    struct stat stStat;
    int iRet = stat(m_szCurLogName, &stStat);

    if(iRet < 0)
    {
        SetErrorNO(EEN_LOGFILE__GET_FILE_STAT_FAILED);

        return -1;
    }

    if(m_stLogConfig.m_iMaxFileSize <= 0)
    {
        m_stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    }

    if(stStat.st_size < m_stLogConfig.m_iMaxFileSize)
    {
        return 0;
    }

    if(m_stLogConfig.m_iMaxFileCount <= 0)
    {
        m_stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
    }

    char szFileName[MAX_FILENAME_LENGTH];
    szFileName[0] = '\0';

    iRet = GetRealLogName(m_stLogConfig.m_iMaxFileCount - 1, szFileName, MAX_FILENAME_LENGTH);

    if(iRet)
    {
        return -2;
    }

    if(access(szFileName, F_OK) == 0)
    {
        if(remove(szFileName) < 0)
        {
            SetErrorNO(EEN_LOGFILE__REMOVE_FILE_FAILED);

            return -3;
        }
    }

    char szRenameFileName[MAX_FILENAME_LENGTH];
    szRenameFileName[0] = '\0';
    int i;
    for(i = m_stLogConfig.m_iMaxFileCount - 2; i >= 0; i--)
    {
        GetRealLogName(i, szFileName, MAX_FILENAME_LENGTH);

        if(access(szFileName, F_OK) == 0)
        {
            GetRealLogName(i+1, szRenameFileName, MAX_FILENAME_LENGTH);

            if(rename(szFileName, szRenameFileName) < 0)
            {
                SetErrorNO(EEN_LOGFILE__RENAME_FILE_FAILED);

                return -4;
            }
        }
    }

    //需要重新打开文件句柄，否则会写入到原inode中
    OpenLogFile();

    return 0;
}

int CLogFile::WriteLogVA(int iLogLevel, const char *pcContent, va_list& ap)
{
    if(CheckWriteLogLevel(iLogLevel))
    {
        return -1;
    }

    if(m_iLogCount >= m_stLogConfig.m_iChkSizeSteps)
    {
        BackupLog();
        m_iLogCount = 0;
    }

    time_t tNow = time(NULL);

    if (tNow - m_tLastCleanupFileTime >= DEFAULT_CHECK_CLEANUP_INTERVAL)
    {
        CleanupLogFile();
        m_tLastCleanupFileTime = tNow;
    }

    if(tNow - m_tLastChkFileTime >= m_stLogConfig.m_iChkFileInterval)
    {
        //文件已经不存在需要重新打开
        if(access(m_szCurLogName, F_OK) != 0)
        {
            OpenLogFile();
        }

        m_tLastChkFileTime = tNow;
    }

    if(m_pFile == NULL)
    {
        OpenLogFile();
    }

    int iCurHour = 0;
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tNow, &stTempTm);
    iCurHour = pTempTm->tm_hour;

    //换小时了可能需要重新打开文件
    if(iCurHour != m_iLastLogHour)
    {
        if(OpenLogFile())
        {
            return -2;
        }

        m_iLastLogHour = iCurHour;
    }

    if(!m_pFile)
    {
        SetErrorNO(EEN_LOGFILE__OPEN_FILE_FAILED);

        return -3;
    }

    char szCurDateTime[MAX_DATETIME_LENGTH];
    CTimeUtility::ConvertUnixTimeToTimeString(tNow, szCurDateTime);

    fprintf(m_pFile, "[%s] ", szCurDateTime);

    fprintf(m_pFile, "[%s] ", GetLogLevelStr(iLogLevel));

    if(m_bEnableFLF && m_pszFile != NULL &&
            m_pszFunc != NULL)
    {
        if(m_iIndentDistance > 0)
        {
            short i;
            for(i = 0; i < m_iIndentDistance && i < MAX_INDENT_DISTANCE; ++i)
            {
                fprintf(m_pFile, "  ");
            }
        }

        const char* pszBaseName = basename(m_pszFile);

        if(pszBaseName == NULL)
        {
            //修改为不显示函数名
            //fprintf(m_pFile, "<%s:%d %s> ", m_pszFile, m_iLine, m_pszFunc);
            fprintf(m_pFile, "<%s:%d> ", m_pszFile, m_iLine);
        }
        else
        {
            //修改为不显示函数名
            //fprintf(m_pFile, "<%s:%d %s> ", pszBaseName, m_iLine, m_pszFunc);
            fprintf(m_pFile, "<%s:%d> ", pszBaseName, m_iLine);
        }
    }

    vfprintf(m_pFile, pcContent, ap);

    if(m_stLogConfig.m_shLogType == ELT_LOG_AND_FFLUSH)
    {
        fflush(m_pFile);
    }

    if(m_stLogConfig.m_shLogType == ELT_LOG_AND_FCLOSE)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    ++m_iLogCount;

    return 0;
}

int CLogFile::CloseFile()
{
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    return 0;
}

int CLogFile::WriteLog(int iLogLevel, const char *pcContent, ...)
{
    va_list ap;
    va_start(ap, pcContent);
    int iRet = WriteLogVA(iLogLevel, pcContent, ap);
    va_end(ap);

    return iRet;
}

int CLogFile::WriteLogExVA(int iLoglevel, int iIndentDistance, const char* pszFile,
                           int iLine, const char* pszFunc, const char *pcContent, va_list& ap)
{
    m_iIndentDistance = iIndentDistance;
    m_pszFile = pszFile;
    m_iLine = iLine;
    m_pszFunc = pszFunc;

    m_bEnableFLF = true;
    int iRet = WriteLogVA(iLoglevel, pcContent, ap);
    m_bEnableFLF = false;

    return iRet;
}

int CLogFile::WriteLogEx(int iLoglevel, int iIndentDistance, const char* pszFile,
                         int iLine, const char* pszFunc, const char *pcContent, ...)
{
    m_iIndentDistance = iIndentDistance;
    m_pszFile = pszFile;
    m_iLine = iLine;
    m_pszFunc = pszFunc;

    va_list ap;
    va_start(ap, pcContent);
    m_bEnableFLF = true;
    int iRet = WriteLogVA(iLoglevel, pcContent, ap);
    m_bEnableFLF = false;
    va_end(ap);

    return iRet;
}

int CLogFile::DumpHex(char *pcBuffer, int iLength)
{
    if(iLength <= 0 || pcBuffer == NULL)
    {
        SetErrorNO(EEN_LOGFILE__NULL_POINTER);

        return -1;
    }

    if(m_iLogCount >= m_stLogConfig.m_iChkSizeSteps)
    {
        BackupLog();
        m_iLogCount = 0;
    }

    time_t tNow = time(NULL);

    if(tNow - m_tLastChkFileTime >= m_stLogConfig.m_iChkFileInterval)
    {
        //文件已经不存在需要重新打开
        if(access(m_szCurLogName, F_OK) != 0)
        {
            OpenLogFile();
        }

        m_tLastChkFileTime = tNow;
    }

    if(m_pFile == NULL)
    {
        OpenLogFile();
    }

    int iCurHour = 0;
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tNow, &stTempTm);
    iCurHour = pTempTm->tm_hour;

    //换小时了可能需要重新打开文件
    if(iCurHour != m_iLastLogHour)
    {
        if(OpenLogFile())
        {
            return -2;
        }

        m_iLastLogHour = iCurHour;
    }

    if(!m_pFile)
    {
        SetErrorNO(EEN_LOGFILE__NULL_POINTER);

        return -2;
    }

    char szCurDateTime[MAX_DATETIME_LENGTH];
    CTimeUtility::ConvertUnixTimeToTimeString(tNow, szCurDateTime);

    if(iLength > MAX_DUMP_HEX_BUF_LENGTH)
    {
        iLength = MAX_DUMP_HEX_BUF_LENGTH;
    }

    fprintf(m_pFile, "[%s] Buffer Length = %d", szCurDateTime, iLength);

    int i;
    for(i = 0; i < iLength; i++)
    {
        if(!(i % 16))
        {
            fprintf(m_pFile, "\n[%s] %04d>\t", szCurDateTime, i / 16 + 1);
        }

        fprintf(m_pFile, "%02X ", (unsigned char)pcBuffer[i]);
    }

    fprintf(m_pFile, "\n");

    if(m_stLogConfig.m_shLogType == ELT_LOG_AND_FFLUSH)
    {
        fflush(m_pFile);
    }

    if(m_stLogConfig.m_shLogType == ELT_LOG_AND_FCLOSE)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }

    return 0;
}

int CLogFile::GetFD() const
{
    return fileno(m_pFile);
}

void CLogFile::CleanupLogFile()
{
    // m_stLogConfig.m_szPath, m_stLogConfig.m_szBaseName

    if (m_stLogConfig.m_iMaxFileExistDays <= 0)
    {
        return;
    }

    DIR *pDir = opendir(m_stLogConfig.m_szPath);
    if (!pDir)
    {
        return;
    }

    time_t tNow = time(NULL);
    time_t tExistTime = m_stLogConfig.m_iMaxFileExistDays * 3600;
    int iLogNameLen = strlen(m_stLogConfig.m_szBaseName);

    struct stat statbuf;
    char szFilePathName[MAX_FILENAME_LENGTH];
    struct dirent* entry = NULL;
    while((entry=readdir(pDir)) != NULL)
    {
        if(strncmp(m_stLogConfig.m_szBaseName, entry->d_name, iLogNameLen))
        {
            continue;
        }

        snprintf(szFilePathName, MAX_FILENAME_LENGTH, "%s/%s", m_stLogConfig.m_szPath, entry->d_name);
        int iRet = lstat(szFilePathName, &statbuf);
        if (iRet != 0)
        {
            break;
        }

        if (tNow - statbuf.st_mtime >= tExistTime)
        {
            unlink(szFilePathName);
        }
    }

    closedir(pDir);
}

const char* CLogFile::GetLogLevelStr(int iLogLevel)
{
    static char szLevelString[32];

    //输出日志级别
    switch(iLogLevel)
    {
    case LOG_LEVEL_DEBUG:
        {
            sprintf(szLevelString, "%s", "DEBUG");
        }
        break;

    case LOG_LEVEL_INFO:
        {
            sprintf(szLevelString, "%s", "INFO");
        }
        break;

    case LOG_LEVEL_DETAIL:
        {
            sprintf(szLevelString, "%s", "DETAIL");
        }
        break;

    case LOG_LEVEL_WARNING:
        {
            sprintf(szLevelString, "%s", "WARNING");
        }
        break;

    case LOG_LEVEL_CALL:
        {
            sprintf(szLevelString, "%s", "CALL");
        }
        break;

    case LOG_LEVEL_ERROR:
        {
            sprintf(szLevelString, "%s", "ERROR");
        }
        break;

    case LOG_LEVEL_ANY:
        {
            sprintf(szLevelString, "%s", "FORCE");
        }
        break;

    default:
        {
            sprintf(szLevelString, "%s", "INVALID");
        }
        break;
    }

    return szLevelString;
}
