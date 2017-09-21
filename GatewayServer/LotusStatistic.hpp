
#ifndef __LOTUS_STATISTIC_HPP__
#define __LOTUS_STATISTIC_HPP__

#include "LotusDefine.hpp"
#include "SingletonTemplate.hpp"



class CLotusStat
{
public:
	void ClearAllStat();

	int RecordAllStat(unsigned int uiIntervalTime);

	inline void AddInputCodeNumber(unsigned int uiCodeNumber)
	{
		m_uiInputCodeNumber += uiCodeNumber;
	};

	inline void AddInputPacket(unsigned int uiPacketLength)
	{
		m_uiInputPacketNumber++;
		m_uiInputPacketLength += uiPacketLength;
	};

	inline void AddOutputPacket(unsigned int uiPacketLength)
	{
		m_uiOutputPacketNumber++;
		m_uiOutputPacketLength += uiPacketLength;
	};

	inline void AddFailedPushNumber(unsigned int uiCodeNumber)
	{
		m_uiFailedPushNumber += uiCodeNumber;
	};

	inline void IncreaseOutputFlashNumber() { ++m_uiOutputFlashNumber; }

	inline void SetMaxPopedCodeNumber(unsigned int uiCodeNumber)
	{
		if(uiCodeNumber > m_uiMaxPopedCodeNumber)
		{
			m_uiMaxPopedCodeNumber = uiCodeNumber;
		}
	};

private:

	//Input
	//Output
	//External
	//Internal
	unsigned int m_uiInputCodeNumber;
	unsigned int m_uiInputPacketNumber;
	unsigned int m_uiInputPacketLength;
	unsigned int m_uiOutputPacketNumber;
	unsigned int m_uiOutputPacketLength;
	unsigned int m_uiOutputFlashNumber;
	unsigned int m_uiFailedPushNumber;
	unsigned int m_uiMaxPopedCodeNumber;
};

typedef Singleton<CLotusStat> LotusStatistic;

#endif
