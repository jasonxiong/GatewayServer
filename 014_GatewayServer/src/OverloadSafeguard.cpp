#include <assert.h>

#include "base.hpp"
#include "OverloadSafeguard.hpp"
#include "ConfigFile.hpp"
#include "LotusLogAdapter.hpp"

COverloadSafeguard::COverloadSafeguard()
{
    m_iRefuseAcceptNum = 0;
    m_iRefuseReceiveNum = 0;
    m_iCurNewCloseConnectNum = 0;
    m_iCurUpPackCounter = 0;
    m_iCurDownPacketCounter = 0;
    m_iCurExternalConnectNum = 0;
    m_iCurNewConnectNum = 0;

	m_pstStrategy = NULL;
}

COverloadSafeguard::~COverloadSafeguard()
{

}

int COverloadSafeguard::Reset()
{
    m_iRefuseAcceptNum = 0;
    m_iRefuseReceiveNum = 0;
    m_iCurNewCloseConnectNum = 0;
    m_iCurUpPackCounter = 0;
    m_iCurDownPacketCounter = 0;
    m_iCurNewConnectNum = 0;

	return 0;
}

bool COverloadSafeguard::IsAcceptConnect()
{
	assert(m_pstStrategy);

	return m_pstStrategy->IsAcceptConnect();
}

bool COverloadSafeguard::IsAcceptReceive()
{
	assert(m_pstStrategy);

	return m_pstStrategy->IsAcceptReceive();
}

bool COverloadSafeguard::IsNeedCloseConnect()
{
	assert(m_pstStrategy);

	return m_pstStrategy->IsNeedCloseConnect();
}

int COverloadSafeguard::Timeout()
{
	if(m_pstStrategy->IsOpenSafeguard())
	{
		//打印统计信息
		TRACESAFEGUARD("===================COverloadSafeguard Stat===================\n");
        TRACESAFEGUARD("RefuseAccept:%d, RefuseReceive:%d CloseConnect:%d\n",  
            m_iRefuseAcceptNum, m_iRefuseReceiveNum, m_iCurNewCloseConnectNum);
		TRACESAFEGUARD("CurUpPackNum:%d, DownPacketNum:%d \n", m_iCurUpPackCounter, m_iCurDownPacketCounter);
		TRACESAFEGUARD("CurExternalConnect:%d, NewConnectNum:%d \n", m_iCurExternalConnectNum, m_iCurNewConnectNum);
		TRACESAFEGUARD("==========================End Stat===========================\n");
	}

	Reset();

	return 0;
}

COverloadSafeguardStrategy::COverloadSafeguardStrategy()
{
	m_pstSafegurad = NULL;
	m_bOpenSafeguard = false;

	m_iConnectCoefficient = 0;
	m_iStableConnectNum = 0;
	m_iNewConnectCoefficient = 0;
	m_iStableNewConnectNum = 0;
	m_iUpPackCoefficient = 0;
	m_iStableUpPackNum = 0;

	m_shStrategyNum = 0;
}

COverloadSafeguardStrategy::~COverloadSafeguardStrategy()
{

}

int COverloadSafeguardStrategy::Initialize(COverloadSafeguard* pstSafeguard, const char* pszStrategyCfg)
{
	m_pstSafegurad = pstSafeguard;

	int iRet = 0;
	ConfigFile stConfigFile;
	iRet = stConfigFile.OpenFile(pszStrategyCfg);
	if(iRet != 0)
	{
		return -1;
	}

	int iOpenSafeguard;
	stConfigFile.GetItemValue("Safeguard", "OpenSafeguard", iOpenSafeguard, 0);
	m_bOpenSafeguard = iOpenSafeguard;

	stConfigFile.GetItemValue("Safeguard", "ConnectCoefficient", m_iConnectCoefficient, 0);
	stConfigFile.GetItemValue("Safeguard", "StableConnectNum", m_iStableConnectNum, 0);

	stConfigFile.GetItemValue("Safeguard", "NewConnectCoefficient", m_iNewConnectCoefficient, 0);
	stConfigFile.GetItemValue("Safeguard", "StableNewConnectNum", m_iStableNewConnectNum, 0);

	stConfigFile.GetItemValue("Safeguard", "UpPackCoefficient", m_iUpPackCoefficient, 0);
	stConfigFile.GetItemValue("Safeguard", "StableUpPackNum", m_iStableUpPackNum, 0);

	int iStrategyNum = 0;
	stConfigFile.GetItemValue("Safeguard", "StrategyNum", iStrategyNum, 0);
	m_shStrategyNum = iStrategyNum;

	if(m_shStrategyNum > MAX_STRATEGY_ITEM_NUM)
	{
		m_shStrategyNum = MAX_STRATEGY_ITEM_NUM;
	}

	TRACESVR(LOG_LEVEL_DETAIL, "COverloadSafeguardStrategy::Initialize, OpenSafeguard:%d, m_iConnectCoefficient:%d, "
		"m_iStableConnectNum:%d, m_iNewConnectCoefficient:%d, m_iStableNewConnectNum:%d"
		"m_iUpPackCoefficient:%d, m_iStableUpPackNum:%d, StrategyNum:%d\n", 
		m_bOpenSafeguard, m_iConnectCoefficient, m_iStableConnectNum, 
		m_iNewConnectCoefficient, m_iStableNewConnectNum, 
		m_iUpPackCoefficient, m_iStableUpPackNum, m_shStrategyNum);

	int i;
	for(i = 0; i < m_shStrategyNum; ++i)
	{
		char szItemName[128];
		sprintf(szItemName, "ItemValue%d", i);
		stConfigFile.GetItemValue("Safeguard", szItemName, m_astStrategy[i].m_iItemValue, 0);
		
		int iTempItem;
		sprintf(szItemName, "AcceptConnect%d", i);
		stConfigFile.GetItemValue("Safeguard", szItemName, iTempItem, 0);
		m_astStrategy[i].m_bAcceptConnect = (bool)iTempItem;
		sprintf(szItemName, "AcceptReceive%d", i);
		stConfigFile.GetItemValue("Safeguard", szItemName, iTempItem, 0);
		m_astStrategy[i].m_bAcceptReceive = (bool)iTempItem;
		sprintf(szItemName, "CloseOldConnect%d", i);
		stConfigFile.GetItemValue("Safeguard", szItemName, iTempItem, 0);
		m_astStrategy[i].m_bNeedCloseConnect = (bool)iTempItem;

		TRACESVR(LOG_LEVEL_DETAIL, "Item[%d]: Value:%d, AcceptConnect:%d, AcceptReceive:%d, CloseOldConnect:%d\n", 
			i, m_astStrategy[i].m_iItemValue, m_astStrategy[i].m_bAcceptConnect, 
			m_astStrategy[i].m_bAcceptReceive, m_astStrategy[i].m_bNeedCloseConnect);
	}

	stConfigFile.CloseFile();

	return 0;
}

void COverloadSafeguardStrategy::CalculateStrategyVal()
{
	assert(m_pstSafegurad);

	//计算负载值
	float fTempValue = (float)m_iConnectCoefficient * (float)m_pstSafegurad->m_iCurExternalConnectNum / (float)m_iStableConnectNum 
		+ (float)m_iNewConnectCoefficient * (float)m_pstSafegurad->m_iCurNewConnectNum / (float)m_iStableNewConnectNum 
		+ (float)m_iUpPackCoefficient * (float)m_pstSafegurad->m_iCurUpPackCounter / (float)m_iStableUpPackNum;

	m_iCurStrategyVal = (int)fTempValue;

	TRACESVR(LOG_LEVEL_DETAIL, "Calculate StrategyVal = %d, CurExternalConnectNum:%d, CurNewConnectNum:%d, CurUpPackCounter:%d\n", 
		m_iCurStrategyVal, m_pstSafegurad->m_iCurExternalConnectNum, m_pstSafegurad->m_iCurNewConnectNum, m_pstSafegurad->m_iCurUpPackCounter);
}

int COverloadSafeguardStrategy::FindStrategyItem(int iStrategyValue)
{
	int i;

	for(i = 0; i < m_shStrategyNum; ++i)
	{
		if(m_astStrategy[i].m_iItemValue > iStrategyValue)
		{
			break;
		}
	}

	if(i == 0)
	{
		return 0;
	}
	else
	{
		return i - 1;
	}
}

bool COverloadSafeguardStrategy::IsAcceptConnect()
{
	//不开启负载保护
	if(m_bOpenSafeguard == false)
	{
		return true;
	}

	//计算当前负载值
	CalculateStrategyVal();

	int iItemIdx = FindStrategyItem(m_iCurStrategyVal);

	bool bAcceptConnect = true;

	if(iItemIdx >= 0 && iItemIdx < m_shStrategyNum)
	{
		bAcceptConnect = m_astStrategy[iItemIdx].m_bAcceptConnect;
	}

	return bAcceptConnect;
}

bool COverloadSafeguardStrategy::IsAcceptReceive()
{
	//不开启负载保护
	if(m_bOpenSafeguard == false)
	{
		return true;
	}

	//计算当前负载值
	CalculateStrategyVal();

	int iItemIdx = FindStrategyItem(m_iCurStrategyVal);

	bool bAcceptReceive = true;

	if(iItemIdx >= 0 && iItemIdx < m_shStrategyNum)
	{
		bAcceptReceive = m_astStrategy[iItemIdx].m_bAcceptReceive;
	}

	return bAcceptReceive;
}

bool COverloadSafeguardStrategy::IsNeedCloseConnect()
{
	//不开启负载保护
	if(m_bOpenSafeguard == false)
	{
		return false;
	}

	//计算当前负载值
	CalculateStrategyVal();

	int iItemIdx = FindStrategyItem(m_iCurStrategyVal);

	bool bNeedCloseConnect = false;

	if(iItemIdx >= 0 && iItemIdx < m_shStrategyNum)
	{
		bNeedCloseConnect = m_astStrategy[iItemIdx].m_bNeedCloseConnect;
	}

	return bNeedCloseConnect;
}
