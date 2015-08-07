/**
*@file MsgStatistic.cpp
*@author jasonxiong
*@date 2009-11-20
*@version 1.0
*@brief
*
*
*/

#include "MsgStatistic.hpp"
#include "LogFile.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

const char* ServerLib::DEFAULT_MSG_STAT_DIRECTORY_PATH = "../log/stat/";
const char* ServerLib::DEFAULT_MSG_STAT_FILE_NAME = "s";

const char* ServerLib::g_apszMsgItemName[ESMI_MAX_ITEM_NUMBER] =
{
    "Success",
    "Fail",
    "Timeout",
    "SumTime",
    "MaxTime",
    "AvaTime",
    "PacketTotalSize"
};

const char* ServerLib::g_apszMsgTypeItemName[ESMTI_MAX_ITEM_NUMBER] =
{
    "Total",
    "Success",
    "Fail",
    "Timeout"
};

const char* ServerLib::g_apszLoopItemName[ESLI_MAX_ITEM_NUMBER] =
{
    "Total"
};

CMsgStatistic::CMsgStatistic()
{
    m_shMsgNum = 0;
    memset(m_astMsgInfo, 0, sizeof(m_astMsgInfo));
    m_iMsgErrorNO = 0;

    m_shMsgTypeNum = 0;
    memset(m_astMsgTypeInfo, 0, sizeof(m_astMsgTypeInfo));

    m_bAddedLoopFlag = false;
}

CMsgStatistic::~CMsgStatistic()
{

}

int CMsgStatistic::Initialize(const char* pszStatPath, const char* pszStatFileName)
{
    TLogConfig stLogConfig;

    if(pszStatPath)
    {
        SAFE_STRCPY(stLogConfig.m_szPath, pszStatPath, sizeof(stLogConfig.m_szPath)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szPath, DEFAULT_MSG_STAT_DIRECTORY_PATH, sizeof(stLogConfig.m_szPath)-1);
    }

    if(pszStatFileName)
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, pszStatFileName, sizeof(stLogConfig.m_szBaseName)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, DEFAULT_MSG_STAT_FILE_NAME, sizeof(stLogConfig.m_szBaseName)-1);
    }

    SAFE_STRCPY(stLogConfig.m_szExtName, ".stat", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    int iRet = m_stMsgStatistic.Initialize("MsgStat", MAX_STAT_MSG_NUMBER, stLogConfig);

    if(iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);

        return -1;
    }

    iRet = m_stMsgTypeStatistic.Initialize("MsgTypeStat", MAX_STAT_MSG_TYPE, stLogConfig);
    if (iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);
        return -2;
    }

    iRet = m_stLoopStatistic.Initialize("LoopStat", MAX_STAT_LOOP_STAT, stLogConfig);
    if (iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);
        return -2;
    }

    m_shMsgNum = 0;
    memset(m_astMsgInfo, 0, sizeof(m_astMsgInfo));

    m_shMsgTypeNum = 0;
    memset(m_astMsgTypeInfo, 0, sizeof(m_astMsgTypeInfo));

    m_bAddedLoopFlag = false;

    return 0;
}

int CMsgStatistic::AddMsgStat(int iMsgID, short shResult, int iMsgLength, timeval tvProcessTime, int iMsgType, int iMsgTimes)
{
    TStatMsgInfo* pstMsgInfo = GetStatMsgInfo(iMsgID);

    if(pstMsgInfo == NULL)
    {
        int iRet = AddMsgInfo(iMsgID);

        if(iRet)
        {
            return -1;
        }

        pstMsgInfo = GetStatMsgInfo(iMsgID);

        //仍然找不到则直接返回失败
        if(pstMsgInfo == NULL)
        {
            SetErrorNO(EEN_MSG_STATISTIC__NULL_POINTER);

            return -2;
        }
    }

    TStatSection* pstStatSection = m_stMsgStatistic.GetSection(pstMsgInfo->m_iMsgIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    switch(shResult)
    {
    case ESMR_SUCCEED:
    {
        pstStatSection->m_astStatItem[ESMI_SUCCESS_MSG_NUMBER_IDX].m_dValue += iMsgTimes;

        break;
    }

    case ESMR_FAILED:
    {
        pstStatSection->m_astStatItem[ESMI_FAILED_MSG_NUMBER_IDX].m_dValue += iMsgTimes;

        break;
    }

    case ESMR_TIMEOUT:
    {
        pstStatSection->m_astStatItem[ESMI_TIMEOUT_MSG_NUMBER_IDX].m_dValue += iMsgTimes;

        break;
    }
    }

    pstStatSection->m_astStatItem[ESMI_MSG_LENGTH].m_dValue += iMsgLength;

    double dProcessUS = (double)tvProcessTime.tv_sec + (double)tvProcessTime.tv_usec / (double)MICROSECOND_PER_SECOND;

    pstStatSection->m_astStatItem[ESMI_SUM_PROCESSTIME_IDX].m_dValue += dProcessUS;

    if(pstStatSection->m_astStatItem[ESMI_MAX_PROCESSTIME_IDX].m_dValue < dProcessUS)
    {
        pstStatSection->m_astStatItem[ESMI_MAX_PROCESSTIME_IDX].m_dValue = dProcessUS;
    }

    int iTotalMsgNum = (int)pstStatSection->m_astStatItem[ESMI_SUCCESS_MSG_NUMBER_IDX].m_dValue +
                       (int)pstStatSection->m_astStatItem[ESMI_FAILED_MSG_NUMBER_IDX].m_dValue +
                       (int)pstStatSection->m_astStatItem[ESMI_TIMEOUT_MSG_NUMBER_IDX].m_dValue;

    pstStatSection->m_astStatItem[ESMI_AVA_PROCESSTIME_IDX].m_dValue =
        pstStatSection->m_astStatItem[ESMI_SUM_PROCESSTIME_IDX].m_dValue /
        iTotalMsgNum;


    AddMsgTypeInfoStat(iMsgType, shResult);

    return 0;
}

int CMsgStatistic::AddMsgTypeInfoStat(int iMsgType, short shResult, int iMsgTimes)
{
    if (iMsgType < 0  || iMsgType >= ESMTI_MAX_ITEM_NUMBER)
    {
        return -1;
    }

    TStatMsgTypeInfo* pstMsgTypeInfo = GetStatMsgTypeInfo(iMsgType);
    if(pstMsgTypeInfo == NULL)
    {
        int iRet = AddMsgTypeInfo(iMsgType);

        if(iRet)
        {
            return -2;
        }

        pstMsgTypeInfo = GetStatMsgTypeInfo(iMsgType);

        //仍然找不到则直接返回失败
        if(pstMsgTypeInfo == NULL)
        {
            SetErrorNO(EEN_MSG_STATISTIC__NULL_POINTER);

            return -3;
        }
    }

    TStatSection* pstStatSection = m_stMsgTypeStatistic.GetSection(pstMsgTypeInfo->m_iMsgTypeIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    switch(shResult)
    {
    case ESMR_SUCCEED:
    {
        pstStatSection->m_astStatItem[ESMTI_TOTAL_SUCCESS_MSG_NUMBER].m_dValue += iMsgTimes;

        break;
    }

    case ESMR_FAILED:
    {
        pstStatSection->m_astStatItem[ESMTI_TOTAL_FAILED_MSG_NUMBER].m_dValue += iMsgTimes;

        break;
    }

    case ESMR_TIMEOUT:
    {
        pstStatSection->m_astStatItem[ESMTI_TOTAL_TIMEOUT_NUMBER].m_dValue += iMsgTimes;

        break;
    }
    }

    pstStatSection->m_astStatItem[ESMTI_TOTAL_MSG_NUMBER].m_dValue += iMsgTimes;

    return 0;
}

TStatMsgInfo* CMsgStatistic::GetStatMsgInfo(int iMsgID)
{
    short i;
    for(i = 0; i < m_shMsgNum; ++i)
    {
        if(m_astMsgInfo[i].m_iMsgID == iMsgID)
        {
            return &m_astMsgInfo[i];
        }
    }

    return NULL;
}

TStatMsgTypeInfo* CMsgStatistic::GetStatMsgTypeInfo(int iMsgType)
{
    short i;
    for(i = 0; i < m_shMsgTypeNum; ++i)
    {
        if(m_astMsgTypeInfo[i].m_iMsgType== iMsgType)
        {
            return &m_astMsgTypeInfo[i];
        }
    }

    return NULL;
}

int CMsgStatistic::AddMsgInfo(int iMsgID)
{
    if(m_shMsgNum >= MAX_STAT_MSG_NUMBER)
    {
        SetErrorNO(EEN_MSG_STATISTIC__MSG_ID_REACH_UPPER_LIMIT);

        return -1;
    }

    char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
    SAFE_SPRINTF(szSectionName, sizeof(szSectionName)-1, "MsgID %d", iMsgID);
    szSectionName[sizeof(szSectionName)-1] = '\0';
    int iSectionIndex = 0;

    int iRet = m_stMsgStatistic.AddSection(szSectionName, iSectionIndex);

    if(iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__ADD_SECTION_FAILED);

        return -2;
    }

    m_astMsgInfo[m_shMsgNum].m_iMsgID = iMsgID;
    m_astMsgInfo[m_shMsgNum].m_iMsgIndex = iSectionIndex;

    TStatSection* pstStatSection = m_stMsgStatistic.GetSection(m_astMsgInfo[m_shMsgNum].m_iMsgIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    pstStatSection->m_shStatItemNum = ESMI_MAX_ITEM_NUMBER;

    //初始化统计项
    short i;
    for(i = 0; i < ESMI_MAX_ITEM_NUMBER; ++i)
    {
        SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                    g_apszMsgItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
    }

    ++m_shMsgNum;

    return 0;
}

int CMsgStatistic::AddMsgTypeInfo(int iMsgType)
{
    if(m_shMsgTypeNum >= MAX_STAT_MSG_TYPE)
    {
        SetErrorNO(EEN_MSG_STATISTIC__MSG_ID_REACH_UPPER_LIMIT);

        return -1;
    }

    char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
    SAFE_SPRINTF(szSectionName, sizeof(szSectionName)-1, "MsgType %d", iMsgType);
    szSectionName[sizeof(szSectionName)-1] = '\0';
    int iSectionIndex = 0;

    int iRet = m_stMsgTypeStatistic.AddSection(szSectionName, iSectionIndex);

    if(iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__ADD_SECTION_FAILED);

        return -2;
    }

    m_astMsgTypeInfo[m_shMsgTypeNum].m_iMsgType = iMsgType;
    m_astMsgTypeInfo[m_shMsgTypeNum].m_iMsgTypeIndex = iSectionIndex;

    TStatSection* pstStatSection = m_stMsgTypeStatistic.GetSection(m_astMsgTypeInfo[m_shMsgTypeNum].m_iMsgTypeIndex);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    pstStatSection->m_shStatItemNum = ESMTI_MAX_ITEM_NUMBER;

    //初始化统计项
    short i;
    for(i = 0; i < ESMTI_MAX_ITEM_NUMBER; ++i)
    {
        SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                    g_apszMsgTypeItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
    }

    ++m_shMsgTypeNum;

    return 0;
}

void CMsgStatistic::Print()
{
    m_stMsgStatistic.Print();
    m_stMsgTypeStatistic.Print();
    m_stLoopStatistic.Print();
}

void CMsgStatistic::Reset()
{
    m_stMsgStatistic.Reset();
    m_stMsgTypeStatistic.Reset();
    m_stLoopStatistic.Reset();
}

int CMsgStatistic::AddLoopStat(int iLoopNumber)
{
    TStatSection* pstStatSection = NULL;

    if (!m_bAddedLoopFlag)
    {
        char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
        SAFE_SPRINTF(szSectionName, sizeof(szSectionName)-1, "Loop");
        szSectionName[sizeof(szSectionName)-1] = '\0';
        int iSectionIndex = 0;

        int iRet = m_stLoopStatistic.AddSection(szSectionName, iSectionIndex);

        if(iRet)
        {
            SetErrorNO(EEN_MSG_STATISTIC__ADD_SECTION_FAILED);

            return -1;
        }

        m_bAddedLoopFlag = true;

        pstStatSection = m_stLoopStatistic.GetSection(iSectionIndex);
        if(pstStatSection == NULL)
        {
            SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

            return -2;
        }

        pstStatSection->m_shStatItemNum = ESLI_MAX_ITEM_NUMBER;

        //初始化统计项
        short i;
        for(i = 0; i < ESLI_MAX_ITEM_NUMBER; ++i)
        {
            SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                        g_apszLoopItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
        }
    }

    pstStatSection = m_stLoopStatistic.GetSection(0);
    if (!pstStatSection)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);
        return -3;
    }

    pstStatSection->m_astStatItem[ESLI_LOOP_NUMBER].m_dValue += iLoopNumber;

    return 0;
}


int CMsgStatistic::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    int iRet = m_stMsgStatistic.Initialize("MsgStat", MAX_STAT_MSG_NUMBER, rstLogConfig);

    if(iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);

        return -1;
    }

    iRet = m_stMsgTypeStatistic.Initialize("MsgTypeStat", MAX_STAT_MSG_TYPE, rstLogConfig);
    if (iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);
        return -2;
    }

    iRet = m_stLoopStatistic.Initialize("LoopStat", MAX_STAT_LOOP_STAT, rstLogConfig);
    if (iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);
        return -2;
    }

    m_shMsgNum = 0;
    memset(m_astMsgInfo, 0, sizeof(m_astMsgInfo));

    m_shMsgTypeNum = 0;
    memset(m_astMsgTypeInfo, 0, sizeof(m_astMsgTypeInfo));

    m_bAddedLoopFlag = false;

    return 0;
}

