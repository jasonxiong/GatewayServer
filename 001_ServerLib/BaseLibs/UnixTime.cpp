/**
*@file UnixTime.cpp
*@author jasonxiong
*@date 2009-11-23
*@version 1.0
*@brief CUnixTime的实现文件
*
*
*/

#include "UnixTime.hpp"
#include "TimeUtility.hpp"

using namespace ServerLib;

CUnixTime::CUnixTime()
{
    m_tTime = time(NULL);
}

CUnixTime::CUnixTime(time_t tTime)
{
    m_tTime = tTime;
}

CUnixTime::~CUnixTime()
{

}

int CUnixTime::GetTimeString(char *szTimeString)
{
    return CTimeUtility::ConvertUnixTimeToTimeString(m_tTime, szTimeString);
}

int CUnixTime::GetDateString(char* szDateString)
{
    return CTimeUtility::ConvertUnixTimeToDateString(m_tTime, szDateString);
}


int CUnixTime::GetMonthString(char* szMonthString)
{
    return CTimeUtility::ConvertUinxTimeToMonthString(m_tTime, szMonthString);
}

int CUnixTime::GetDaysAfterYear(int iYear, unsigned short& rushDays)
{
    return CTimeUtility::ConvertUnixTimeToDaysAfterYear(m_tTime, iYear, rushDays);
}

int CUnixTime::GetYear(int& iYear)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&m_tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -1;
    }

    iYear = pTempTm->tm_year + 1900;

    return 0;
}

int CUnixTime::GetYear()
{
    int iYear = 0;
    GetYear(iYear);

    return iYear;
}

int CUnixTime::GetMonth(int& iMonth)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&m_tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -1;
    }

    iMonth = pTempTm->tm_mon + 1;

    return 0;
}

int CUnixTime::GetMonth()
{
    int iMonth = 0;
    GetMonth(iMonth);

    return iMonth;
}

int CUnixTime::GetDate(int& iDate)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&m_tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -1;
    }

    iDate = pTempTm->tm_mday;

    return 0;
}

int CUnixTime::GetDate()
{
    int iDate = 0;
    GetDate(iDate);

    return iDate;
}

int CUnixTime::GetHour(int& riHour)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&m_tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -1;
    }

    riHour = pTempTm->tm_hour;

    return 0;
}

int CUnixTime::GetHour()
{
    int iHour = 0;
    GetHour(iHour);

    return iHour;
}

int CUnixTime::GetMinute(int& riMinute)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&m_tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -1;
    }

    riMinute = pTempTm->tm_min;

    return 0;
}

int CUnixTime::GetMinute()
{
    int iMinute = 0;
    GetMinute(iMinute);

    return iMinute;
}

void CUnixTime::RefreshTime()
{
    m_tTime = time(NULL);
}


