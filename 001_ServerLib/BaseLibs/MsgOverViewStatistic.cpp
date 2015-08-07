#include "LogAdapter.hpp"
#include "MsgOverViewStatistic.hpp"

using namespace ServerLib;

CMsgOverViewStatistic::CMsgOverViewStatistic(void)
{
    m_iMsgNumberFromClient = 0;
    m_iMsgSizeFromClient = 0;

    m_iMsgNumberFromServer = 0;
    m_iMsgSizeFromServer = 0;

    m_iMsgNumberToClient = 0;
    m_iMsgSizeToClient = 0;

    m_iMsgNumberToServer = 0;
    m_iMsgSizeToServer = 0;

    m_iLoopNumber = 0;
}

CMsgOverViewStatistic::~CMsgOverViewStatistic(void)
{
}

void CMsgOverViewStatistic::ClearAllStat()
{
    m_iMsgNumberFromClient = 0;
    m_iMsgSizeFromClient = 0;

    m_iMsgNumberFromServer = 0;
    m_iMsgSizeFromServer = 0;

    m_iMsgNumberToClient = 0;
    m_iMsgSizeToClient = 0;

    m_iMsgNumberToServer = 0;
    m_iMsgSizeToServer = 0;

    m_iLoopNumber = 0;
}


int CMsgOverViewStatistic::RecordAllStat(int iIntervalTime)
{
    TRACESVR("IntervalTime: %d LoopNumber: %d\nMsgNumberFromClient: %d MsgSizeFromClient: %d\nMsgNumberFromServer: %d MsgSizeFromServer: %d\n"
             "MsgNumberToClient: %d MsgSizeToClient: %d\nMsgNumberToServer: %d MsgSizeToServer: %d\n",
             iIntervalTime, m_iLoopNumber, m_iMsgNumberFromClient, m_iMsgSizeFromClient, m_iMsgNumberFromServer,
             m_iMsgSizeFromServer, m_iMsgNumberToClient, m_iMsgSizeToClient, m_iMsgNumberToServer, m_iMsgSizeToServer);

    return 0;
}

