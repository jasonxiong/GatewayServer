/**
*@file CacheStatistic.cpp
*@author jasonxiong
*@date 2009-11-21
*@version 1.0
*@brief Cache统计类实现文件
*
*
*/

#include "CacheStatistic.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

const char* ServerLib::DEFAULT_CACHE_STAT_DIRECTORY_PATH = "../log/stat/";
const char* ServerLib::DEFAULT_CACHE_STAT_FILE_NAME = "s";

const char* ServerLib::g_apszCacheStatusItemName[ECSI_MAX_ITEM_NUMBER] =
{
    "Used",
    "Free",
    "Recycled",
    "Swap",
};

const char* ServerLib::g_apszReadCacheItemName[ERCI_MAX_ITEM_NUMBER] =
{
    "Hit",
    "Miss",
    "HitRate",
};

const char* ServerLib::g_apszWriteCacheItemName[EWCI_MAX_ITEM_NUMBER] =
{
    "Hit",
    "Miss",
    "HitRate",
};


CCacheStatistic::CCacheStatistic()
{
    m_shCacheTypeNum = 0;
    memset(m_astCacheInfo, 0, sizeof(m_astCacheInfo));
    m_iErrorNO = 0;
}

CCacheStatistic::~CCacheStatistic()
{

}

int CCacheStatistic::Initialize(const char* pszStatPath, const char* pszStatFileName)
{
    TLogConfig stLogConfig;

    if(pszStatPath)
    {
        SAFE_STRCPY(stLogConfig.m_szPath, pszStatPath, sizeof(stLogConfig.m_szPath)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szPath, DEFAULT_CACHE_STAT_DIRECTORY_PATH, sizeof(stLogConfig.m_szPath)-1);
    }

    if(pszStatFileName)
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, pszStatFileName, sizeof(stLogConfig.m_szBaseName)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, DEFAULT_CACHE_STAT_FILE_NAME, sizeof(stLogConfig.m_szBaseName)-1);
    }

    SAFE_STRCPY(stLogConfig.m_szExtName, ".stat", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    int iRet = m_stStatistic.Initialize("CacheStat", MAX_CACHE_TYPE_NUMBER * 3, stLogConfig);

    if(iRet)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__STATISTIC_INIT_FAILED);

        return -1;
    }

    m_shCacheTypeNum = 0;
    memset(m_astCacheInfo, 0, sizeof(m_astCacheInfo));

    return 0;
}

int CCacheStatistic::AddCacheStatusSection(const char* pszCacheName, int& riSectionIdx)
{
    if(pszCacheName == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__NULL_POINTER);

        return -1;
    }

    char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
    SAFE_SPRINTF(szSectionName, sizeof(szSectionName)-1, "%s", pszCacheName);
    szSectionName[sizeof(szSectionName)-1] = '\0';
    int iSectionIndex = 0;

    int iRet = m_stStatistic.AddSection(szSectionName, iSectionIndex);

    if(iRet)
    {
        riSectionIdx = -1;
        SetErrorNO(EEN_CACHE_STATISTIC__ADD_SECTION_FAILED);

        return -2;
    }

    TStatSection* pstStatSection = m_stStatistic.GetSection(iSectionIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    pstStatSection->m_shStatItemNum = ECSI_MAX_ITEM_NUMBER;
    riSectionIdx = iSectionIndex;

    //初始化统计项
    short i;
    for(i = 0; i < ECSI_MAX_ITEM_NUMBER; ++i)
    {
        SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                    g_apszCacheStatusItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
    }

    return 0;
}

int CCacheStatistic::AddReachCacheSection(const char* pszCacheName, int& riSectionIdx)
{
    if(pszCacheName == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__NULL_POINTER);

        return -1;
    }

    char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
    SAFE_SPRINTF(szSectionName, sizeof(szSectionName)-1, "Read %s", pszCacheName);
    szSectionName[sizeof(szSectionName)-1] = '\0';
    int iSectionIndex = 0;

    int iRet = m_stStatistic.AddSection(szSectionName, iSectionIndex);

    if(iRet)
    {
        riSectionIdx = -1;
        SetErrorNO(EEN_CACHE_STATISTIC__ADD_SECTION_FAILED);

        return -2;
    }

    TStatSection* pstStatSection = m_stStatistic.GetSection(iSectionIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    pstStatSection->m_shStatItemNum = ERCI_MAX_ITEM_NUMBER;
    riSectionIdx = iSectionIndex;

    //初始化统计项
    short i;
    for(i = 0; i < ERCI_MAX_ITEM_NUMBER; ++i)
    {
        SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                    g_apszReadCacheItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
    }

    return 0;
}

int CCacheStatistic::AddWriteCacheSection(const char* pszCacheName, int& riSectionIdx)
{
    if(pszCacheName == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__NULL_POINTER);

        return -1;
    }

    char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
    SAFE_SPRINTF(szSectionName, sizeof(szSectionName)-1, "Write %s", pszCacheName);
    szSectionName[sizeof(szSectionName)-1] = '\0';
    int iSectionIndex = 0;

    int iRet = m_stStatistic.AddSection(szSectionName, iSectionIndex);

    if(iRet)
    {
        riSectionIdx = -1;
        SetErrorNO(EEN_CACHE_STATISTIC__ADD_SECTION_FAILED);

        return -2;
    }

    TStatSection* pstStatSection = m_stStatistic.GetSection(iSectionIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    pstStatSection->m_shStatItemNum = EWCI_MAX_ITEM_NUMBER;
    riSectionIdx = iSectionIndex;

    //初始化统计项
    short i;
    for(i = 0; i < EWCI_MAX_ITEM_NUMBER; ++i)
    {
        SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                    g_apszWriteCacheItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
    }

    return 0;
}

int CCacheStatistic::AddCacheType(int iCacheType, const char* pszCacheName)
{
    if(pszCacheName == NULL)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__NULL_POINTER);

        return -1;
    }

    if(m_shCacheTypeNum >= MAX_CACHE_TYPE_NUMBER)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__STATISTIC_INIT_FAILED);

        return -2;
    }

    int iCacheStatusIndex = -1;
    int iRet = AddCacheStatusSection(pszCacheName, iCacheStatusIndex);

    if(iRet)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__ADD_SECTION_FAILED);

        return -3;
    }

    int iReadCacheIndex = -1;
    iRet = AddReachCacheSection(pszCacheName, iReadCacheIndex);

    if(iRet)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__ADD_SECTION_FAILED);

        return -4;
    }

    int iWriteCacheIndex = -1;
    iRet = AddWriteCacheSection(pszCacheName, iWriteCacheIndex);

    if(iRet)
    {
        SetErrorNO(EEN_CACHE_STATISTIC__ADD_SECTION_FAILED);

        return -5;
    }

    m_astCacheInfo[m_shCacheTypeNum].m_iCacheType = iCacheType;
    m_astCacheInfo[m_shCacheTypeNum].m_iCacheStatusIndex = iCacheStatusIndex;
    m_astCacheInfo[m_shCacheTypeNum].m_iReadCacheIndex = iReadCacheIndex;
    m_astCacheInfo[m_shCacheTypeNum].m_iWriteCacheIndex = iWriteCacheIndex;

    ++m_shCacheTypeNum;

    return 0;
}

TStatCacheInfo* CCacheStatistic::GetCacheInfo(int iCacheType)
{
    short i;
    for(i = 0; i < m_shCacheTypeNum; ++i)
    {
        if(m_astCacheInfo[i].m_iCacheType == iCacheType)
        {
            return &m_astCacheInfo[i];
        }
    }

    return NULL;
}

int CCacheStatistic::UpdateCacheStatus(int iCacheType, int iUsedCount, int iFreeCount, int iRecycleCount, int iSwapTimes)
{
    TStatCacheInfo* pstCacheInfo = GetCacheInfo(iCacheType);

    if(pstCacheInfo == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__NULL_POINTER);

        return -1;
    }

    TStatSection* pstStatSection = m_stStatistic.GetSection(pstCacheInfo->m_iCacheStatusIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -2;
    }

    pstStatSection->m_astStatItem[ECSI_USED_COUNT].m_dValue = iUsedCount;
    pstStatSection->m_astStatItem[ECSI_FREE_COUNT].m_dValue = iFreeCount;
    pstStatSection->m_astStatItem[ECSI_RECYCLE_COUNT].m_dValue = iRecycleCount;
    pstStatSection->m_astStatItem[ECSI_SWAP_TIMES].m_dValue = iSwapTimes;

    return 0;
}

int CCacheStatistic::AddReadCacheStat(int iCacheType, int iAddHitCount, int iAddMissCount)
{
    TStatCacheInfo* pstCacheInfo = GetCacheInfo(iCacheType);

    if(pstCacheInfo == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__NULL_POINTER);

        return -1;
    }

    TStatSection* pstStatSection = m_stStatistic.GetSection(pstCacheInfo->m_iReadCacheIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -2;
    }

    pstStatSection->m_astStatItem[ERCI_HIT_CACHE_COUNT].m_dValue += iAddHitCount;
    pstStatSection->m_astStatItem[ERCI_MISS_CACHE_COUNT].m_dValue += iAddMissCount;
    pstStatSection->m_astStatItem[ERCI_HIT_RATE].m_dValue = pstStatSection->m_astStatItem[ERCI_HIT_CACHE_COUNT].m_dValue * 100 /
            (pstStatSection->m_astStatItem[ERCI_HIT_CACHE_COUNT].m_dValue +
             pstStatSection->m_astStatItem[ERCI_MISS_CACHE_COUNT].m_dValue);

    return 0;
}

int CCacheStatistic::AddWriteCacheStat(int iCacheType, int iAddHitCount, int iAddMissCount)
{
    TStatCacheInfo* pstCacheInfo = GetCacheInfo(iCacheType);

    if(pstCacheInfo == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__NULL_POINTER);

        return -1;
    }

    TStatSection* pstStatSection = m_stStatistic.GetSection(pstCacheInfo->m_iWriteCacheIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -2;
    }

    pstStatSection->m_astStatItem[EWCI_HIT_CACHE_COUNT].m_dValue += iAddHitCount;
    pstStatSection->m_astStatItem[EWCI_MISS_CACHE_COUNT].m_dValue += iAddMissCount;
    pstStatSection->m_astStatItem[EWCI_HIT_RATE].m_dValue = pstStatSection->m_astStatItem[EWCI_HIT_CACHE_COUNT].m_dValue * 100 /
            (pstStatSection->m_astStatItem[EWCI_HIT_CACHE_COUNT].m_dValue +
             pstStatSection->m_astStatItem[EWCI_MISS_CACHE_COUNT].m_dValue);

    return 0;
}

void CCacheStatistic::Print()
{
    m_stStatistic.Print();
}

void CCacheStatistic::Reset()
{
    m_stStatistic.Reset();
}

