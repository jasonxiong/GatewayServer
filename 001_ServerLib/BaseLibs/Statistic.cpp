/**
*@file Statistic.cpp
*@author jasonxiong
*@date 2009-11-20
*@version 1.0
*@brief
*
*
*/

#include "Statistic.hpp"
#include "NowTime.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

CStatistic::CStatistic()
{
    m_iMaxSectionNum = 0;
    m_iCurSectionNum = 0;
    m_astStatSection = NULL;
    m_tLastReset = NowTimeSingleton::Instance()->GetNowTime();
}

CStatistic::~CStatistic()
{
    if(m_astStatSection)
    {
        delete []m_astStatSection;
    }
}

int CStatistic::Initialize(const char* pszStatName, int iMaxSectionNum)
{
    TLogConfig stLogConfig;

    SAFE_STRCPY(stLogConfig.m_szPath, "../log/stat/", sizeof(stLogConfig.m_szPath)-1);
    SAFE_STRCPY(stLogConfig.m_szBaseName, "s", sizeof(stLogConfig.m_szBaseName)-1);
    SAFE_STRCPY(stLogConfig.m_szExtName, ".stat", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    return Initialize(pszStatName, iMaxSectionNum, stLogConfig);
}

int CStatistic::Initialize(const char* pszStatName, int iMaxSectionNum, const TLogConfig& rstLogConfig)
{
    if(iMaxSectionNum <= 0)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_SECTION_NUMBER);

        return -1;
    }

    if(pszStatName == NULL)
    {
        SetErrorNO(EEN_STATISTIC__NULL_POINTER);

        return -2;
    }

    SAFE_STRCPY(m_szStatName, pszStatName, sizeof(m_szStatName)-1);
    m_szStatName[sizeof(m_szStatName)-1] = '\0';

    int iRet = m_stLogFile.Initialize(rstLogConfig);

    if(iRet)
    {
        SetErrorNO(EEN_STATISTIC__LOG_FILE_INIT_FAILED);

        return -3;
    }

    if(m_astStatSection)
    {
        delete []m_astStatSection;
    }

    m_astStatSection = new TStatSection[iMaxSectionNum];

    if(m_astStatSection == NULL)
    {
        m_iMaxSectionNum = 0;
        m_iCurSectionNum = 0;

        SetErrorNO(EEN_STATISTIC__NEW_SECTION_FAILED);

        return -4;
    }

    memset(m_astStatSection, 0, sizeof(TStatSection)*iMaxSectionNum);
    m_iMaxSectionNum = iMaxSectionNum;
    m_iCurSectionNum = 0;

    return 0;
}

int CStatistic::AddItem(int iSectionIdx, const char* pszItemName, int& riItemIdx)
{
    if(pszItemName == NULL)
    {
        SetErrorNO(EEN_STATISTIC__NULL_POINTER);

        return -1;
    }

    TStatSection* pstStatSection = GetSection(iSectionIdx);

    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_STATISTIC__NULL_POINTER);

        return -2;
    }

    if(pstStatSection->m_shStatItemNum >= MAX_STAT_ITEM_NUMBER)
    {
        SetErrorNO(EEN_STATISTIC__ITEM_NUMBER_REACH_UPPER_LIMIT);

        return -3;
    }

    TStatItem& rstStatItem = pstStatSection->m_astStatItem[pstStatSection->m_shStatItemNum];
    SAFE_STRCPY(rstStatItem.m_szName, pszItemName, sizeof(rstStatItem.m_szName)-1);
    rstStatItem.m_dValue = 0;
    riItemIdx = pstStatSection->m_shStatItemNum;
    ++pstStatSection->m_shStatItemNum;

    return 0;
}

bool CStatistic::IsDoubleInt(double dValue)
{
    return dValue == (int)dValue;
}

int CStatistic::SetValue(int iSectionIdx, int iItemIdx, double dValue)
{
    TStatSection* pstSection = GetSection(iSectionIdx);

    if(pstSection == NULL)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_SECTION_INDEX);

        return -1;
    }

    if(iItemIdx < 0 || iItemIdx >= pstSection->m_shStatItemNum)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_ITEM_INDEX);

        return -2;
    }

    pstSection->m_astStatItem[iItemIdx].m_dValue = dValue;

    return 0;
}

int CStatistic::AddValue(int iSectionIdx, int iItemIdx, double dPlusValue)
{
    TStatSection* pstSection = GetSection(iSectionIdx);

    if(pstSection == NULL)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_SECTION_INDEX);

        return -1;
    }

    if(iItemIdx < 0 || iItemIdx >= pstSection->m_shStatItemNum)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_ITEM_INDEX);

        return -2;
    }

    pstSection->m_astStatItem[iItemIdx].m_dValue += dPlusValue;

    return 0;
}

int CStatistic::AddSection(const char* pszSectionName, int& riSectionIdx)
{
    if(pszSectionName == NULL)
    {
        SetErrorNO(EEN_STATISTIC__NULL_POINTER);

        return -1;
    }

    if(m_iCurSectionNum >= m_iMaxSectionNum)
    {
        SetErrorNO(EEN_STATISTIC__SECTION_NUMBER_REACH_UPPER_LIMIT);

        return -2;
    }

    TStatSection& rstSection = m_astStatSection[m_iCurSectionNum];
    memset(&rstSection, 0, sizeof(rstSection));
    SAFE_STRCPY(rstSection.m_szName, pszSectionName, sizeof(rstSection.m_szName)-1);
    rstSection.m_szName[sizeof(rstSection.m_szName)-1] = '\0';
    riSectionIdx = m_iCurSectionNum;
    ++m_iCurSectionNum;

    return 0;
}

TStatSection* CStatistic::GetSection(int iSectionIdx)
{
    if(iSectionIdx < 0 || iSectionIdx >= m_iCurSectionNum)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_SECTION_INDEX);

        return NULL;
    }

    return &m_astStatSection[iSectionIdx];
}

void CStatistic::Reset()
{
    int i;
    for(i = 0; i < m_iCurSectionNum; ++i)
    {
        short j;
        for(j = 0; j < m_astStatSection[i].m_shStatItemNum; ++j)
        {
            m_astStatSection[i].m_astStatItem[j].m_dValue = 0;
        }
    }

    m_tLastReset = NowTimeSingleton::Instance()->GetNowTime();
}

void CStatistic::Print()
{
    time_t tNow = NowTimeSingleton::Instance()->GetNowTime();

    m_stLogFile.WriteLog(LOG_LEVEL_ANY, "----------------------%s Begin---------------------\n",
                         m_szStatName);

    m_stLogFile.WriteLog(LOG_LEVEL_ANY, "Stat: Interval = %d\n",
                         tNow - m_tLastReset);

    char szStatSection[MAX_STAT_SECTION_STRING_LENGTH];
    char szTemp[256];

    int i;
    for(i = 0; i < m_iCurSectionNum; ++i)
    {
        if(BIT_DISABLED(m_astStatSection[i].m_iStatFlag, ESSF_NOT_PRINT))
        {
            SAFE_SPRINTF(szStatSection, sizeof(szStatSection)-1, "%s: ", m_astStatSection[i].m_szName);

            short j;
            for(j = 0; j < m_astStatSection[i].m_shStatItemNum; ++j)
            {
                TStatItem& rstItem = m_astStatSection[i].m_astStatItem[j];

                if(IsDoubleInt(rstItem.m_dValue))
                {
                    SAFE_SPRINTF(szTemp, sizeof(szTemp)-1, "%s = %d", rstItem.m_szName,
                                 (int)rstItem.m_dValue);
                }
                else
                {
                    SAFE_SPRINTF(szTemp, sizeof(szTemp)-1, "%s = %.6f", rstItem.m_szName,
                                 rstItem.m_dValue);
                }

                strncat(szStatSection, szTemp, sizeof(szStatSection)-1);

                if(j != m_astStatSection[i].m_shStatItemNum - 1)
                {
                    strncat(szStatSection, ", ", sizeof(szStatSection)-1);
                }
            }

            m_stLogFile.WriteLog(LOG_LEVEL_ANY, "%s\n", szStatSection);
        }
    }

    m_stLogFile.WriteLog(LOG_LEVEL_ANY, "----------------------%s End--------------------------\n", m_szStatName);
    m_stLogFile.CloseFile(); //由于多个统计类都打在同一个统计文件上，因此打印后关闭文件
}
