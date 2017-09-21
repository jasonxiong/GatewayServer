
#include "MsgStatistic.hpp"
#include "ErrorDef.hpp"

using namespace DeprecatedLib;

const char* g_apszMsgItemName[ESMI_MAX_ITEM_NUM] = 
{
	"Success", 
	"Fail", 
	"Timeout", 
	"SumTime", 
	"MaxTime", 
	"AvaTime"
};

CMsgStatistic::CMsgStatistic()
{
	m_shMsgNum = 0;
	memset(m_astMsgInfo, 0, sizeof(m_astMsgInfo));
	m_iErrorNO = 0;
}

CMsgStatistic::~CMsgStatistic()
{

}

int CMsgStatistic::Initialize(const char* pszStatPath, const char* pszStatFileName)
{
	int iRet = m_stStatistic.Initialize("LotusServer Msg Stat", MAX_STAT_MSG_NUM);
	if(iRet)
	{
		SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);
		return -1;
	}

	m_shMsgNum = 0;
	memset(m_astMsgInfo, 0, sizeof(m_astMsgInfo));

	return 0;
}

int CMsgStatistic::AddMsgStat(int iMsgID, short shResult, timeval tvProcessTime)
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

	TStatSection* pstStatSection = m_stStatistic.GetSection(pstMsgInfo->m_iMsgIndex);

	if(pstStatSection == NULL)
	{
		SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

		return -3;
	}

	switch(shResult)
	{
	case ESMR_SUCCEED:
		{
			++pstStatSection->m_astStatItem[ESMI_SUCCESS_MSG_NUM_IDX].m_dValue;

			break;
		}

	case ESMR_FAILED:
		{
			++pstStatSection->m_astStatItem[ESMI_FAILED_MSG_NUM_IDX].m_dValue;

			break;
		}

	case ESMR_TIMEOUT:
		{
			++pstStatSection->m_astStatItem[ESMI_TIMEOUT_MSG_NUM_IDX].m_dValue;

			break;
		}
	}

	double dProcessUS = (double)tvProcessTime.tv_sec + (double)tvProcessTime.tv_usec / (double)1000000;

	pstStatSection->m_astStatItem[ESMI_SUM_PROCESSTIME_IDX].m_dValue += dProcessUS;
	
	if(pstStatSection->m_astStatItem[ESMI_MAX_PROCESSTIME_IDX].m_dValue < dProcessUS)
	{
		pstStatSection->m_astStatItem[ESMI_MAX_PROCESSTIME_IDX].m_dValue = dProcessUS;
	}

	int iTotalMsgNum = (int)pstStatSection->m_astStatItem[ESMI_SUCCESS_MSG_NUM_IDX].m_dValue + 
		(int)pstStatSection->m_astStatItem[ESMI_FAILED_MSG_NUM_IDX].m_dValue + 
		(int)pstStatSection->m_astStatItem[ESMI_TIMEOUT_MSG_NUM_IDX].m_dValue;

	pstStatSection->m_astStatItem[ESMI_AVA_PROCESSTIME_IDX].m_dValue = 
		pstStatSection->m_astStatItem[ESMI_SUM_PROCESSTIME_IDX].m_dValue / 
		iTotalMsgNum;

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

int CMsgStatistic::AddMsgInfo(int iMsgID)
{
	if(m_shMsgNum >= MAX_STAT_MSG_NUM)
	{
		SetErrorNO(EEN_MSG_STATISTIC__MSG_ID_REACH_UPPER_LIMIT);
		
		return -1;
	}

	char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
	snprintf(szSectionName, sizeof(szSectionName)-1, "MsgID %d", iMsgID);
	szSectionName[sizeof(szSectionName)-1] = '\0';
	int iSectionIndex = 0;

	int iRet = m_stStatistic.AddSection(szSectionName, iSectionIndex);

	if(iRet)
	{
		SetErrorNO(EEN_MSG_STATISTIC__ADD_SECTION_FAILED);

		return -2;
	}

	m_astMsgInfo[m_shMsgNum].m_iMsgID = iMsgID;
	m_astMsgInfo[m_shMsgNum].m_iMsgIndex = iSectionIndex;

	TStatSection* pstStatSection = m_stStatistic.GetSection(m_astMsgInfo[m_shMsgNum].m_iMsgIndex);

	if(pstStatSection == NULL)
	{
		SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

		return -3;
	}

	pstStatSection->m_shStatItemNum = ESMI_MAX_ITEM_NUM;

	//初始化统计项
	short i;
	for(i = 0; i < ESMI_MAX_ITEM_NUM; ++i)
	{
		strncpy(pstStatSection->m_astStatItem[i].m_szName, 
			g_apszMsgItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
	}

	++m_shMsgNum;

	return 0;
}

void CMsgStatistic::Print()
{
	m_stStatistic.Print();
}

void CMsgStatistic::Reset()
{
	m_stStatistic.Reset();
}
