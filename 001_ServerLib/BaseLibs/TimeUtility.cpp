/**
*@file TimeUtility.cpp
*@author jasonxiong
*@date 2009-11-23
*@version 1.0
*@brief Time相关操作的实现文件
*
*
*/

//在这添加标准库头文件
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

//在这添加ServerLib头文件
#include "CommonDef.hpp"
#include "TimeUtility.hpp"
#include "StringUtility.hpp"
#include "LogAdapter.hpp"

using namespace ServerLib;

CTimeValue CTimeUtility::m_stTimeValueTick;
time_t CTimeUtility::m_uiTimeTick = 0;

int CTimeUtility::ConvertUnixTimeToTimeString(time_t tTime, char* szTimeString)
{
    if(szTimeString == NULL)
    {
        return -1;
    }

    szTimeString[0] = '\0';

    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -2;
    }

    sprintf(szTimeString, "%04d-%02d-%02d %02d:%02d:%02d",
            pTempTm->tm_year + 1900, pTempTm->tm_mon + 1, pTempTm->tm_mday,
            pTempTm->tm_hour, pTempTm->tm_min, pTempTm->tm_sec);

    return 0;
}

int CTimeUtility::ConvertUnixTimeToDateString(time_t tTime, char* szDateString)
{
    if(szDateString == NULL)
    {
        return -1;
    }

    szDateString[0] = '\0';

    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -2;
    }

    sprintf(szDateString, "%04d-%02d-%02d",
            pTempTm->tm_year + 1900, pTempTm->tm_mon + 1, pTempTm->tm_mday);

    return 0;
}

int CTimeUtility::ConvertUinxTimeToMonthString(time_t tTime, char* szMonthString)
{
    if(szMonthString == NULL)
    {
        return -1;
    }

    szMonthString[0] = '\0';

    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -2;
    }

    sprintf(szMonthString, "%04d-%02d",
            pTempTm->tm_year + 1900, pTempTm->tm_mon + 1);

    return 0;
}

int CTimeUtility::ConvertTimeStringToUnixTime(char* szTimeString, time_t& rtTime)
{
    if(szTimeString == NULL)
    {
        return -1;
    }

    struct tm stTempTime;

    int iRet = ConvertTimeStringToTmStruct(szTimeString, stTempTime);

    if(iRet)
    {
        return -2;
    }

    rtTime = mktime(&stTempTime);

    return 0;
}

int CTimeUtility::ConvertTimeStringToTmStruct(char* szTimeString, struct tm& rstTM)
{
    if(szTimeString == NULL || strlen(szTimeString) < (size_t)MIN_TIME_STRING_LENGTH)
    {
        return -1;
    }

    memset(&rstTM, 0, sizeof(rstTM));

    char szTempTime[MIN_TIME_STRING_LENGTH+1];
    SAFE_STRCPY(szTempTime, szTimeString, sizeof(szTempTime)-1);

    char* pTime = &szTempTime[0];

    //取出年
    pTime[4] = '\0';
    rstTM.tm_year = atoi(pTime) - 1900;
    pTime += 5;

    pTime[2] = '\0';
    rstTM.tm_mon = atoi(pTime) -1;
    pTime += 3;

    pTime[2] = '\0';
    rstTM.tm_mday = atoi(pTime);
    pTime += 3;

    pTime[2] = '\0';
    rstTM.tm_hour = atoi(pTime);
    pTime += 3;

    pTime[2] = '\0';
    rstTM.tm_min = atoi(pTime);
    pTime += 3;

    pTime[2] = '\0';
    rstTM.tm_sec = atoi(pTime);
    pTime += 3;

    return 0;
}

int CTimeUtility::ConvertUnixTimeToDaysAfterYear(time_t tTime, int iYear, unsigned short& rushDays)
{
    if(iYear < 2000 || iYear > 2100)
    {
        return -1;
    }

    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(!pTempTm)
    {
        return -2;
    }

    //设为那年的1月1日
    stTempTm.tm_year = iYear - 1900;
    stTempTm.tm_yday = 0;
    stTempTm.tm_mon = 0;
    stTempTm.tm_mday = 1;

    time_t tYear = 0;
    tYear = mktime(&stTempTm);

    //rushDays = (stTempTm.tm_year + 1900 - iYear) * 366 + stTempTm.tm_yday;
    rushDays = (tTime - tYear) / 86400;

    return 0;
}

int CTimeUtility::TimeValPlus(const timeval& tvA, const timeval& tvB, timeval& tvResult)
{
    tvResult.tv_sec = tvA.tv_sec + tvB.tv_sec;
    tvResult.tv_sec += ((tvA.tv_usec + tvB.tv_usec) / 1000000);
    tvResult.tv_usec = (tvA.tv_usec + tvB.tv_usec) % 1000000;

    return 0;
}

int CTimeUtility::TimeValMinus(const timeval& tvA, const timeval& tvB, timeval& tvResult)
{
    if(tvA.tv_usec < tvB.tv_usec)
    {
        tvResult.tv_usec = (1000000 + tvA.tv_usec) - tvB.tv_usec;
        tvResult.tv_sec = tvA.tv_sec - tvB.tv_sec - 1;
    }
    else
    {
        tvResult.tv_usec = tvA.tv_usec - tvB.tv_usec;
        tvResult.tv_sec  = tvA.tv_sec - tvB.tv_sec;
    }

    return 0;
}

time_t CTimeUtility::GetTodayTime(time_t tTime, int iTimeSeconds)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(!pTempTm)
    {
        return tTime;
    }

    stTempTm.tm_sec = 0;
    stTempTm.tm_min = 0;
    stTempTm.tm_hour = 0;

    return mktime(&stTempTm) + iTimeSeconds;
}



time_t CTimeUtility::GetWeekTime(time_t tTime, int iWeekDay, int iDaySeconds)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(!pTempTm)
    {
        return tTime;
    }

    // 获得当天的0点
    stTempTm.tm_sec = 0;
    stTempTm.tm_min = 0;
    stTempTm.tm_hour = 0;

    time_t stRetTime = mktime(&stTempTm);

    //星期天由0变换为7
    if(stTempTm.tm_wday == 0)
    {
        stTempTm.tm_wday = 7;
    }

    // 获得iWeekDay的0点
    stRetTime += (iWeekDay - stTempTm.tm_wday) * 24 * 3600;

    // 获得iWeekDay的iDaySeconds时间值
    stRetTime += iDaySeconds;

    return stRetTime;
}


int CTimeUtility::GetWeekDay(time_t tTime)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(!pTempTm)
    {
        return tTime;
    }

    stTempTm.tm_sec = 0;
    stTempTm.tm_min = 0;
    stTempTm.tm_hour = 0;

    //time_t stRetTime = mktime(&stTempTm);

    //星期天由0变换为7
    if(stTempTm.tm_wday == 0)
    {
        stTempTm.tm_wday = 7;
    }

    return stTempTm.tm_wday;
}

int CTimeUtility::GetLastNearestHourTime(time_t tTime, int iHour)
{
    return GetNextNearestHourTime(tTime, iHour) - 24 * 60 * 60;
}

int CTimeUtility::GetNextNearestHourTime(time_t tTime, int iHour)
{
    int iTodayZero = CTimeUtility::GetTodayTime(tTime);

    int iDeltaHour = (tTime - iTodayZero) / 3600;//小时

    if (iDeltaHour < iHour)
    {
        return iTodayZero + iHour * 60 * 60;
    }
    else
    {
        return iTodayZero + (iHour + 24) * 60 * 60;
    }
}

int CTimeUtility::GetNextNearestSecTime(time_t tTime, int iSec)
{
    int iTodayZero = CTimeUtility::GetTodayTime(tTime);

    int iDeltaSec = (tTime - iTodayZero);

    if (iDeltaSec < iSec)
    {
        return iTodayZero + iSec;
    }
    else
    {
        return iTodayZero + iSec + 24 * 3600;
    }
}


//获得若干个指定整点数里，最近的一个下次刷新时间
int CTimeUtility::GetNextNearestMutiHourTimes(time_t tTime, const MultiHours* pstMultiHours)
{
    if (pstMultiHours->m_cHoursNum <= 0)
    {
        return 0;
    }

    int iMinTimes = GetNextNearestHourTime(tTime, pstMultiHours->m_acHour[0]);
    for (int i = 1; i < pstMultiHours->m_cHoursNum; i++)
    {
        int iTimes = GetNextNearestHourTime(tTime, pstMultiHours->m_acHour[i]);
        if (iMinTimes > iTimes)
        {
            iMinTimes = iTimes;
        }
    }

    return iMinTimes;
}

// 返回年月日，时分秒，星期
int CTimeUtility::GetTimeOfDay(time_t tTime,
                               int* iYear, int* iMon, int* iDay, int* iHour, int* iMin, int* iSec, int* iWeekDay)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);
    if(pTempTm == NULL)
    {
        TRACESVR("Localtime_r failed: %s\n", strerror(errno));
        return -1;
    }

    if (iYear)
    {
        *iYear = pTempTm->tm_year + 1900;
    }
    if (iMon)
    {
        *iMon = pTempTm->tm_mon + 1;
    }
    if (iDay)
    {
        *iDay = pTempTm->tm_mday;
    }
    if (iHour)
    {
        *iHour = pTempTm->tm_hour;
    }
    if (iMin)
    {
        *iMin = pTempTm->tm_min;
    }
    if (iSec)
    {
        *iSec = pTempTm->tm_sec;
    }
    if (iWeekDay)
    {
        *iWeekDay = pTempTm->tm_wday;
        if (*iWeekDay == 0)
        {
            *iWeekDay = 7;
        }
    }

    return 0;
}

time_t CTimeUtility::MakeTime(int iYear, int iMonth, int iDay, int iHour, int iMin, int iSec)
{
    struct tm stCompareTime;
    stCompareTime.tm_year = iYear - 1900;
    stCompareTime.tm_mon = iMonth - 1;
    stCompareTime.tm_mday = iDay;
    stCompareTime.tm_hour = iHour;
    stCompareTime.tm_min = iMin;
    stCompareTime.tm_sec = iSec;

    time_t iCompareTime = mktime(&stCompareTime);

    return iCompareTime;
}


int CTimeUtility::CompareDateTime(time_t tTime, int iYear, int iMonth, int iDay, int iHour, int iMin, int iSec)
{
    time_t iCompareTime = MakeTime(iYear, iMonth, iDay, iHour, iMin, iSec);

    if (tTime > iCompareTime)
    {
        return 1;
    }

    if (tTime < iCompareTime)
    {
        return -1;
    }

    return 0;
}

int CTimeUtility::CompareTime(time_t tTime, int iHour, int iMin, int iSec)
{
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tTime, &stTempTm);

    if(pTempTm == NULL)
    {
        return -1;
    }

    int iYear = pTempTm->tm_year + 1900;
    int iMon = pTempTm->tm_mon + 1;
    int iDay = pTempTm->tm_mday;

    return CompareDateTime(tTime, iYear, iMon, iDay, iHour, iMin, iSec);
}

int CTimeUtility::GetNextNearestWeekTime(time_t tTime, int iWeekDay, int iDaySeconds)
{
    time_t tWeekTime = CTimeUtility::GetWeekTime(tTime, iWeekDay, iDaySeconds);
    if (tTime <= tWeekTime)
    {
        return tWeekTime;
    }
    else
    {
        return tWeekTime + 7 * 24 * 3600;
    }
}

int CTimeUtility::GetLastNearestWeekTime(time_t tTime, int iWeekDay, int iDaySeconds)
{
    return GetNextNearestWeekTime(tTime, iWeekDay, iDaySeconds) - 7 * 24 * 3600;
}

bool CTimeUtility::IsInSameMonth(time_t tTime1, time_t tTime2)
{
    struct tm stTempTm1;
    struct tm *pTempTm1 = localtime_r(&tTime1, &stTempTm1);
    if(pTempTm1 == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    struct tm stTempTm2;
    struct tm *pTempTm2 = localtime_r(&tTime2, &stTempTm2);
    if(pTempTm2 == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    if (pTempTm1->tm_year != pTempTm2->tm_year)
    {
        return false;
    }

    if (pTempTm1->tm_mon != pTempTm2->tm_mon)
    {
        return false;
    }

    return true;
}

bool CTimeUtility::IsInSameYear(time_t tTime1, time_t tTime2)
{
    struct tm stTempTm1;
    struct tm *pTempTm1 = localtime_r(&tTime1, &stTempTm1);
    if(pTempTm1 == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    struct tm stTempTm2;
    struct tm *pTempTm2 = localtime_r(&tTime2, &stTempTm2);
    if(pTempTm2 == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    return (pTempTm1->tm_year == pTempTm2->tm_year);
}

bool CTimeUtility::IsInSameDay(time_t tTime1, time_t tTime2)
{
    struct tm stTempTm1;
    struct tm *pTempTm1 = localtime_r(&tTime1, &stTempTm1);
    if(pTempTm1 == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    struct tm stTempTm2;
    struct tm *pTempTm2 = localtime_r(&tTime2, &stTempTm2);
    if(pTempTm2 == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    if (pTempTm1->tm_year != pTempTm2->tm_year)
    {
        return false;
    }

    if (pTempTm1->tm_mon != pTempTm2->tm_mon)
    {
        return false;
    }

    if (pTempTm1->tm_mday != pTempTm2->tm_mday)
    {
        return false;
    }

    return true;
}

//是否在同一周
bool CTimeUtility::IsInSameWeek(time_t tTime1, time_t tTime2)
{
    time_t tMinTime = tTime1<tTime2 ? tTime1 : tTime2;
    time_t tMaxTime = tTime1<tTime2 ? tTime2 : tTime1;
    struct tm stTempTm;
    struct tm *pTempTm = localtime_r(&tMinTime, &stTempTm);
    if(pTempTm == NULL)
    {
        LOGDEBUG("Localtime_r failed: %s\n", strerror(errno));
        return false;
    }

    //获取一周的截至时间
    pTempTm->tm_mday += (6-pTempTm->tm_wday);
    pTempTm->tm_hour = 23;
    pTempTm->tm_min = 59;
    pTempTm->tm_sec = 59;

    //再转化为绝对时间
    time_t tTmpTime = mktime(pTempTm);

    if(tMaxTime > tTmpTime)
    {
        return false;
    }

    return true;
}

//获取下一天0点的时间
time_t CTimeUtility::GetNextDayBeginTime()
{
    time_t tTmpTime = GetNowTime();
    struct tm stTempTm;
    struct tm* pTempTm = localtime_r(&tTmpTime, &stTempTm);
    if(!pTempTm)
    {
        return tTmpTime;
    }

    pTempTm->tm_mday += 1;
    pTempTm->tm_hour = 0;
    pTempTm->tm_min = 0;
    pTempTm->tm_sec = 0;

    return mktime(pTempTm);
}
