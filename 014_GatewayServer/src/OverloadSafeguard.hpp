
#ifndef __OVERLOAD_SAFEGUARD_HPP__
#define __OVERLOAD_SAFEGUARD_HPP__

class COverloadSafeguard;

typedef struct tagSafeguardStrategyItem
{
	int m_iItemValue; //阀值
	bool m_bAcceptConnect; 
	bool m_bAcceptReceive;
	bool m_bNeedCloseConnect;

} TSafeguardStrategyItem;

const int MAX_STRATEGY_ITEM_NUM = 10;

class COverloadSafeguardStrategy
{
public:
	COverloadSafeguardStrategy();
	virtual ~COverloadSafeguardStrategy();

	virtual int Initialize(COverloadSafeguard* pstSafeguard, const char* pszStrategyCfg);

	virtual bool IsAcceptConnect();
	virtual bool IsAcceptReceive();
	virtual bool IsNeedCloseConnect();

	bool IsOpenSafeguard() { return m_bOpenSafeguard; }

	void CalculateStrategyVal();
	int FindStrategyItem(int iStrategyValue);

private:
	COverloadSafeguard* m_pstSafegurad;

	bool m_bOpenSafeguard;

	int m_iConnectCoefficient; //当前连接系数
	int m_iStableConnectNum; //稳定连接个数

	int m_iNewConnectCoefficient; //新连接系数
	int m_iStableNewConnectNum; //稳定新连接个数

	int m_iUpPackCoefficient; //上行包量系数
	int m_iStableUpPackNum; //稳定上行包量数

	int m_iCurStrategyVal; //当前策略值

	short m_shStrategyNum; //处理策略个数
	TSafeguardStrategyItem m_astStrategy[MAX_STRATEGY_ITEM_NUM];
};

class COverloadSafeguard
{
public:
	COverloadSafeguard();
	~COverloadSafeguard();

	void SetStrategy(COverloadSafeguardStrategy* pstStrategy) { m_pstStrategy = pstStrategy; }
	int Reset();

	bool IsAcceptConnect();
	bool IsAcceptReceive();
	bool IsNeedCloseConnect();

	void IncreaseRefuseAcceptNum() { ++m_iRefuseAcceptNum; }
	void IncreaseRefuseReciveNum() { ++m_iRefuseReceiveNum; }
    void IncreaseCloseNewConnectNum() { ++m_iCurNewCloseConnectNum; }
    
	void IncreaseUpPacketNum() { ++m_iCurUpPackCounter; }
	void IncreaseDownPacketNum() { ++m_iCurDownPacketCounter;}

	void IncreaseExternalConnectNum() { ++m_iCurExternalConnectNum; }
	void DecreaseExternalConnectNum() { --m_iCurExternalConnectNum; }

    void IncreaseNewConnectNum() { ++m_iCurNewConnectNum; }

	int Timeout();

public:
	//由于防过载受限制统计
	int m_iRefuseAcceptNum; //拒绝的Accept
	int m_iRefuseReceiveNum; //拒绝的Receive
    int m_iCurNewCloseConnectNum; //当前关闭连接数

	int m_iCurUpPackCounter; //当前的上行包量
	int m_iCurDownPacketCounter; //当前的下行包量

	int m_iCurExternalConnectNum; //当前外部连接数
	int m_iCurNewConnectNum; //当前的新连接数
	
	COverloadSafeguardStrategy* m_pstStrategy;
};

#endif

