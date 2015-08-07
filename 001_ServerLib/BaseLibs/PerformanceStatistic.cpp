/**
*@file PerformanceStatistic.cpp
*@author jasonxiong
*@date 2009-12-21
*@version 1.0
*@brief 操作统计类的实现文件
*
*
*/

#include "Statistic.hpp"
#include "PerformanceStatistic.hpp"
#include "CommonDef.hpp"
#include "LogFile.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

const char* ServerLib::g_apszOPItemName[ESOI_MAX_ITEM_NUMBER] =
{
    "CallTimes",
    "SumTime",
    "AverageTime",
    "MaxTime"
};

CPerformanceStatistic::CPerformanceStatistic()
{
    m_iErrorNO = 0;
}

CPerformanceStatistic::~CPerformanceStatistic()
{

}

int CPerformanceStatistic::Initialize(const char* pszStatPath /* = NULL */, const char* pszStatFileName /* = NULL */)
{
    TLogConfig stLogConfig;

    if(pszStatPath)
    {
        SAFE_STRCPY(stLogConfig.m_szPath, pszStatPath, sizeof(stLogConfig.m_szPath)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szPath, "../log/stat/", sizeof(stLogConfig.m_szPath)-1);
    }

    if(pszStatFileName)
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, pszStatFileName, sizeof(stLogConfig.m_szBaseName)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, "s", sizeof(stLogConfig.m_szBaseName)-1);
    }

    SAFE_STRCPY(stLogConfig.m_szExtName, ".stat", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;
    m_iErrorNO = 0;

    int iRet = m_stStatistic.Initialize("Performance", MAX_PERFORMANCE_STAT_OP_NUMBER, stLogConfig);

    if(iRet)
    {
        SetErrorNO(EEN_PERFORMANCE_STAT__STATISTIC_INIT_FAILED);

        return -1;
    }

    return 0;
}

int CPerformanceStatistic::AddOPStat(const char* pszOPName, timeval tvProcessTime)
{
    if(pszOPName == NULL)
    {
        SetErrorNO(EEN_PERFORMANCE_STAT__NULL_POINTER);

        return -1;
    }

    TStatSection* pstStatSection = NULL;

    short i;
    for(i = 0; i < m_stStatistic.GetSectionNum(); ++i)
    {
        TStatSection* pstTempSection = m_stStatistic.GetSection(i);

        if(pstTempSection && strcmp(pstTempSection->m_szName, pszOPName) == 0)
        {
            pstStatSection = pstTempSection;

            break;
        }
    }

    //未找到则添加
    if(pstStatSection == NULL)
    {
        int iSectionIdx = 0;
        int iRet = m_stStatistic.AddSection(pszOPName, iSectionIdx);

        if(iRet)
        {
            SetErrorNO(EEN_PERFORMANCE_STAT__SECTION_NUMBER_REACH_UPPER_LIMIT);

            return -2;
        }

        pstStatSection = m_stStatistic.GetSection(iSectionIdx);

        if(pstStatSection == NULL)
        {
            SetErrorNO(EEN_PERFORMANCE_STAT__NULL_POINTER);

            return -3;
        }

        pstStatSection->m_shStatItemNum = ESOI_MAX_ITEM_NUMBER;

        //初始化统计项
        short i;
        for(i = 0; i < ESOI_MAX_ITEM_NUMBER; ++i)
        {
            SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName, g_apszOPItemName[i],
                        sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
        }
    }

    //更新统计值
    ++pstStatSection->m_astStatItem[ESOI_OP_CALL_TIMES].m_dValue;
    double dProcessUS = (double)tvProcessTime.tv_sec + (double)tvProcessTime.tv_usec / (double)MICROSECOND_PER_SECOND;
    pstStatSection->m_astStatItem[ESOI_SUM_PROCESS_TIME].m_dValue += dProcessUS;

    if(pstStatSection->m_astStatItem[ESOI_MAX_PROCESS_TIME].m_dValue < dProcessUS)
    {
        pstStatSection->m_astStatItem[ESOI_MAX_PROCESS_TIME].m_dValue = dProcessUS;
    }

    int iTotalOPNum = (int)pstStatSection->m_astStatItem[ESOI_OP_CALL_TIMES].m_dValue;

    pstStatSection->m_astStatItem[ESOI_AVA_PROCESS_TIME].m_dValue =
        pstStatSection->m_astStatItem[ESOI_SUM_PROCESS_TIME].m_dValue /
        iTotalOPNum;

    return 0;
}

void CPerformanceStatistic::Print()
{
    m_stStatistic.Print();
}

void CPerformanceStatistic::Reset()
{
    m_iErrorNO = 0;
    m_stStatistic.Reset();
}
