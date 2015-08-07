/**
*@file TimeValue.cpp
*@author jasonxiong
*@date 2009-11-24
*@version 1.0
*@brief CTimeValue的实现文件
*
*
*/

#include "TimeValue.hpp"
#include "TimeUtility.hpp"

using namespace ServerLib;

CTimeValue::CTimeValue()
{
}

CTimeValue::CTimeValue(timeval tvTimeVal)
{
    m_stTimeval = tvTimeVal;
}

CTimeValue::CTimeValue(int iSec, int iUsec)
{
    m_stTimeval.tv_sec = iSec;
    m_stTimeval.tv_usec = iUsec;
}

CTimeValue::~CTimeValue()
{

}

void CTimeValue::RefreshTime()
{
    gettimeofday(&m_stTimeval, NULL);
}

CTimeValue& CTimeValue::operator=(const CTimeValue& rstTime)
{
    m_stTimeval = rstTime.m_stTimeval;

    return *this;
}

CTimeValue CTimeValue::operator+(const CTimeValue& rstTime)
{
    timeval stTimeValResult;

    CTimeUtility::TimeValPlus(m_stTimeval, rstTime.m_stTimeval, stTimeValResult);
    CTimeValue stTimeManager;
    stTimeManager.SetTimeValue(stTimeValResult);

    return stTimeManager;
}

CTimeValue CTimeValue::operator-(const CTimeValue& rstTime)
{
    timeval stTimeValResult;

    CTimeUtility::TimeValMinus(m_stTimeval, rstTime.m_stTimeval, stTimeValResult);
    CTimeValue stTimeManager;
    stTimeManager.SetTimeValue(stTimeValResult);

    return stTimeManager;
}

int CTimeValue::operator>(const CTimeValue& rstTime)
{
    return (m_stTimeval.tv_sec > rstTime.m_stTimeval.tv_sec ||
            (m_stTimeval.tv_sec == rstTime.m_stTimeval.tv_sec && m_stTimeval.tv_usec > rstTime.m_stTimeval.tv_usec));
}

int CTimeValue::operator<(const CTimeValue& rstTime)
{
    return (m_stTimeval.tv_sec < rstTime.m_stTimeval.tv_sec ||
            (m_stTimeval.tv_sec == rstTime.m_stTimeval.tv_sec && m_stTimeval.tv_usec < rstTime.m_stTimeval.tv_usec));
}

int CTimeValue::operator>=(const CTimeValue& rstTime)
{
    return (m_stTimeval.tv_sec > rstTime.m_stTimeval.tv_sec ||
            (m_stTimeval.tv_sec == rstTime.m_stTimeval.tv_sec && m_stTimeval.tv_usec >= rstTime.m_stTimeval.tv_usec));
}

int CTimeValue::operator<=(const CTimeValue& rstTime)
{
    return (m_stTimeval.tv_sec < rstTime.m_stTimeval.tv_sec ||
            (m_stTimeval.tv_sec == rstTime.m_stTimeval.tv_sec && m_stTimeval.tv_usec <= rstTime.m_stTimeval.tv_usec));
}

int CTimeValue::operator==(const CTimeValue& rstTime)
{
    return (m_stTimeval.tv_sec == rstTime.m_stTimeval.tv_sec &&
            m_stTimeval.tv_usec == rstTime.m_stTimeval.tv_usec);
}

int CTimeValue::operator!=(const CTimeValue& rstTime)
{
    return (m_stTimeval.tv_sec != rstTime.m_stTimeval.tv_sec ||
            m_stTimeval.tv_usec != rstTime.m_stTimeval.tv_usec);
}

