/**
*@file NowTime.cpp
*@author jasonxiong
*@date 2009-11-23
*@version 1.0
*@brief CNowTime的实现文件
*
*
*/

#include <time.h>
#include <sys/time.h>

#include "NowTime.hpp"

using namespace ServerLib;

time_t CNowTime::GetNowTime() const
{
    if(m_bUsePseudoTime)
    {
        return m_stPseudoTime.GetTime();
    }
    else
    {
        return time(NULL);
    }
}

timeval CNowTime::GetNowTimeVal() const
{
    if(m_bUsePseudoTime)
    {
        return m_stPseudoTimeValue.GetTimeValue();
    }
    else
    {
        timeval tvTimeVal;
        gettimeofday(&tvTimeVal, NULL);
        return tvTimeVal;
    }
}
