
#include "Statistic.hpp"
#include "ErrorDef.hpp"
#include "LotusLogAdapter.hpp"
#include "base.hpp"
using namespace DeprecatedLib;

CStatistic::CStatistic()
{
	m_iMaxSectionNum = 0;
	m_iCurSectionNum = 0;
	m_astStatSection = NULL;
	m_tLastReset = time(NULL);
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
    if (pszStatName)
    {
        snprintf(m_szStatName, MAX_STAT_SECTION_NAME_LENGTH, "%s", pszStatName);
    }
    else
    {
        snprintf(m_szStatName, MAX_STAT_SECTION_NAME_LENGTH, "%s", "stat");
    }

    if(iMaxSectionNum <= 0)
    {
        SetErrorNO(EEN_STATISTIC__INVALID_SECTION_NUM);

        return -1;
    }

    int iRet = 0;

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

    memset(m_astStatSection, 0, sizeof(m_astStatSection));
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

	if(pstStatSection->m_shStatItemNum >= MAX_STAT_ITEM_NUM)
	{
		SetErrorNO(EEN_STATISTIC__ITEM_NUM_REACH_UPPER_LIMIT);

		return -3;
	}

	TStatItem& rstStatItem = pstStatSection->m_astStatItem[pstStatSection->m_shStatItemNum];
	strncpy(rstStatItem.m_szName, pszItemName, sizeof(rstStatItem.m_szName)-1);
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
		SetErrorNO(EEN_STATISTIC__SECTION_NUM_REACH_UPPER_LIMIT);

		return -2;
	}

	TStatSection& rstSection = m_astStatSection[m_iCurSectionNum];
	memset(&rstSection, 0, sizeof(rstSection));
	strncpy(rstSection.m_szName, pszSectionName, sizeof(rstSection.m_szName)-1);
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

	m_tLastReset = time(NULL);
}

void CStatistic::Print()
{
	time_t tNow = time(NULL);

	TRACESTAT("----------------------%s begin---------------------\n", 
		m_szStatName);

	TRACESTAT("Stat: Interval = %d\n", 
		tNow - m_tLastReset);

	char szStatSection[MAX_STAT_SECTION_STRING_LENGTH];
	char szTemp[256];

	int i;
	for(i = 0; i < m_iCurSectionNum; ++i)
	{
		if(BIT_DISABLED(m_astStatSection[i].m_iStatFlag, ESSF_NOT_PRINT))
		{
			snprintf(szStatSection, sizeof(szStatSection)-1, "%s: ", m_astStatSection[i].m_szName);

			short j;
			for(j = 0; j < m_astStatSection[i].m_shStatItemNum; ++j)
			{
				TStatItem& rstItem = m_astStatSection[i].m_astStatItem[j];

				if(IsDoubleInt(rstItem.m_dValue))
				{
					snprintf(szTemp, sizeof(szTemp)-1, "%s = %d", rstItem.m_szName, 
						(int)rstItem.m_dValue);
				}
				else
				{
					snprintf(szTemp, sizeof(szTemp)-1, "%s = %.6f", rstItem.m_szName, 
						rstItem.m_dValue);
				}

				strncat(szStatSection, szTemp, sizeof(szStatSection)-1);

				if(j != m_astStatSection[i].m_shStatItemNum - 1)
				{
					strncat(szStatSection, ", ", sizeof(szStatSection)-1);
				}
			}

			TRACESTAT("%s\n", szStatSection);
		}
	}

	TRACESTAT("---------------------------end stat--------------------------\n");
}
