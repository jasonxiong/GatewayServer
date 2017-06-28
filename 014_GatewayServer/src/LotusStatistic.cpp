
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

#ifdef ENABLE_TNM2_REPORT
    TNM2_REPORT_VALUE(10532, m_uiInputPacketNumber);
    TNM2_REPORT_VALUE(10628, m_uiOutputPacketNumber);
    TNM2_REPORT_VALUE(10629, m_uiInputPacketLength);
    TNM2_REPORT_VALUE(13536, m_uiOutputPacketLength);
    TNM2_REPORT_VALUE(13539, 0);
    TNM2_REPORT_VALUE(13544, m_uiFailedPushNumber);
#endif

    return 0;
}
