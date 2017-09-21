
#include "LotusStatistic.hpp"
#include "LotusLogAdapter.hpp"
#include <string.h>

void CLotusStat::ClearAllStat()
{
    m_uiOutputFlashNumber = 0;
    m_uiInputCodeNumber = 0;
    m_uiInputPacketNumber = 0;
    m_uiInputPacketLength = 0;
    m_uiOutputPacketNumber = 0;
    m_uiOutputPacketLength = 0;
    m_uiFailedPushNumber = 0;
    m_uiMaxPopedCodeNumber = 0;
}

int CLotusStat::RecordAllStat(unsigned int uiIntervalTime)
{
    TRACESTAT("IntervalTime:%u InputCodeNumber:%u InputPacketNumber:%u InputPacketLength:%u "
        "FailedPushNumber:%u OutputPacketNumber:%u OutputPacketLength:%u "
        "MaxPopedCodeNumber:%u OutputFlashNumber:%d\n",
        uiIntervalTime, m_uiInputCodeNumber, m_uiInputPacketNumber, m_uiInputPacketLength,
        m_uiFailedPushNumber, m_uiOutputPacketNumber, m_uiOutputPacketLength,
        m_uiMaxPopedCodeNumber, m_uiOutputFlashNumber);

    return 0;
}
