#include <stdio.h>
#include <stdlib.h>
#include "TimeTools.hpp"
#include "base.hpp"

static timeval g_tvBeginTime;

int GetTimeStringOfDay(char *strTime)
{
	struct tm *pTempTm = NULL;
	time_t timer;

	if( !strTime )
	{
		return -1;
	}

	time( &timer );

	strTime[0] = '\0';

	pTempTm = localtime( &timer );

	if( !pTempTm )
	{
		return -1;
	}

	sprintf(strTime, "%02d:%02d:%02d", pTempTm->tm_hour, pTempTm->tm_min, pTempTm->tm_sec);

	return 0;
}

int GetDateString(char *strTime)
{
	struct tm *pTempTm = NULL;
	time_t timer;

	if( !strTime )
	{
		return -1;
	}

	time( &timer );

	strTime[0] = '\0';

	pTempTm = localtime( &timer );

	if( !pTempTm )
	{
		return -1;
	}

	sprintf(strTime, "%04d-%02d-%02d", pTempTm->tm_year + 1900, pTempTm->tm_mon + 1, pTempTm->tm_mday);

	return 0;
}

int GetStructTime(time_t tTime, TStruTime *pstTime)
{
	struct tm *pTempTm = NULL;
	struct tm stTempTm;
	time_t timer = tTime;

	if( !pstTime )
	{
		return -1;
	}

	pTempTm = localtime_r( &timer, &stTempTm );

	if( !pTempTm )
	{
		return -1;
	}

	pstTime->m_iYear = stTempTm.tm_year + 1900;
	pstTime->m_iMon = stTempTm.tm_mon + 1;
	pstTime->m_iDay = stTempTm.tm_mday;
	pstTime->m_iHour = stTempTm.tm_hour;
	pstTime->m_iMin = stTempTm.tm_min;
	pstTime->m_iSec = stTempTm.tm_sec;
	pstTime->m_iMSec = 0;

	return 0;
}

int GetCurStructTime(TStruTime *pstTime)
{
	time_t tTempNow;

	time( &tTempNow );

	return GetStructTime(tTempNow, pstTime);
}

int GetTimeString( time_t timer, char *strTime)
{
	struct tm *pTempTm = NULL;
	struct tm stTempTm;

	if( !strTime )
	{
		return -1;
	}

	strTime[0] = '\0';

	pTempTm = localtime_r( &timer, &stTempTm );

	if( !pTempTm )
	{
		return -1;
	}

	sprintf(strTime, "%04d-%02d-%02d %02d:%02d:%02d", stTempTm.tm_year + 1900, stTempTm.tm_mon + 1, stTempTm.tm_mday, stTempTm.tm_hour, stTempTm.tm_min, stTempTm.tm_sec);

	return 0;
}


int TimeValPlus(timeval& tvA, timeval& tvB, timeval& tvResult)
{
	timeval tvTemp;
	tvTemp.tv_sec = tvA.tv_sec + tvB.tv_sec;
	tvTemp.tv_sec += ((tvA.tv_usec+tvB.tv_usec)/1000000);
	tvTemp.tv_usec = ((tvA.tv_usec+tvB.tv_usec)%1000000);

	tvResult.tv_sec = tvTemp.tv_sec;
	tvResult.tv_usec = tvTemp.tv_usec;

	return 0;
}

int TimeValMinus(timeval& tvA, timeval& tvB, timeval& tvResult)
{
	timeval tvTemp;

	if( tvA.tv_usec < tvB.tv_usec )
	{
		tvTemp.tv_usec = (1000000 + tvA.tv_usec) - tvB.tv_usec;
		tvTemp.tv_sec = tvA.tv_sec - tvB.tv_sec - 1;
	}
	else
	{
		tvTemp.tv_usec = tvA.tv_usec - tvB.tv_usec;
		tvTemp.tv_sec  = tvA.tv_sec - tvB.tv_sec;
	}

	tvResult.tv_sec = tvTemp.tv_sec;
	tvResult.tv_usec = tvTemp.tv_usec;

	return 0;
}

int GetTStruTimeFromString(char *strTime, TStruTime &stTime)
{
	char *pTime = NULL;
	

	if (NULL == strTime) 
	{
		return -1;
	}
	
	pTime = &strTime[0];
	
	//取出年
	pTime[4] = '\0';
	stTime.m_iYear = atoi(pTime);
	pTime += 5;

	pTime[2] = '\0';
	stTime.m_iMon= atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTime.m_iDay= atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTime.m_iHour= atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTime.m_iMin= atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTime.m_iSec= atoi(pTime);
	pTime += 3;	

	return 0;
}

void BeginGetTimeVal()
{
	gettimeofday( &g_tvBeginTime, NULL );		//测量处理时长--开始
}
 
void EndGetTimeVal(timeval& tvVal)
{
	timeval tvTempEnd;

	gettimeofday( &tvTempEnd, NULL );		//测量处理时长--结束
	if( tvTempEnd.tv_usec < g_tvBeginTime.tv_usec )	{
		tvVal.tv_usec = (1000000 + tvTempEnd.tv_usec) - g_tvBeginTime.tv_usec;
		tvVal.tv_sec = tvTempEnd.tv_sec - g_tvBeginTime.tv_sec - 1;
	}	else {
		tvVal.tv_usec = tvTempEnd.tv_usec - g_tvBeginTime.tv_usec;
		tvVal.tv_sec = tvTempEnd.tv_sec - g_tvBeginTime.tv_sec;
	}
}


// 起始：增加人: 曾宇   日期: 2005-2-25
CDayTime::CDayTime()
{
	m_iHour = 0;
	m_iMinute = 0;
	m_iSecond = 0;
}
CDayTime::CDayTime(time_t tSecTime)
{
	struct tm stTempTm;
	localtime_r(&tSecTime, &stTempTm);
	SetHMS(stTempTm.tm_hour, stTempTm.tm_min, stTempTm.tm_sec);
}
CDayTime::CDayTime(int iHour, int iMinute, int iSecond)
{
	SetHMS(iHour, iMinute, iSecond);
}
CDayTime::~CDayTime()
{
}
CDayTime& CDayTime::operator =(const CDayTime& rDayTime)
{
	m_iHour = rDayTime.m_iHour;
	m_iMinute = rDayTime.m_iMinute;
	m_iSecond = rDayTime.m_iSecond;
	return *this;
}
int CDayTime::operator ==(const CDayTime& rDayTime) const
{
	return (m_iHour == rDayTime.m_iHour && m_iMinute == rDayTime.m_iMinute && m_iSecond == rDayTime.m_iSecond);
}
int CDayTime::operator !=(const CDayTime& rDayTime) const
{
	return !(*this == rDayTime);
}
int CDayTime::operator >(const CDayTime& rDayTime) const
{
	if(m_iHour > rDayTime.m_iHour)
	{
		return true;
	}
	else if(m_iHour < rDayTime.m_iHour)
	{
		return false;
	}
	else if(m_iMinute > rDayTime.m_iMinute)
	{
		return true;
	}
	else if(m_iMinute < rDayTime.m_iMinute)
	{
		return false;
	}
	else if(m_iSecond > rDayTime.m_iSecond)
	{
		return true;
	}
	else
	{
		return false;
	}
}
int CDayTime::operator <(const CDayTime& rDayTime) const
{
	return !(*this > rDayTime) && !(*this == rDayTime);
}
int CDayTime::operator >=(const CDayTime& rDayTime) const
{
	return !(*this < rDayTime);
}
int CDayTime::operator <=(const CDayTime& rDayTime) const
{
	return !(*this > rDayTime);
}
void CDayTime::GetHMS(int& riH, int& riM, int &riS)
{
	riH = m_iHour;
	riM = m_iMinute;
	riS = m_iSecond;
}
void CDayTime::GetString(char *szTString)
{
	if(szTString)
	{
		sprintf(szTString, "%02d:%02d:%02d", m_iHour, m_iMinute, m_iSecond);
	}
}
int CDayTime::SetHMS(int iHour, int iMinute, int iSecond)
{
	if(!IsHourValid(iHour) || !IsMinuteValid(iMinute) || !IsSecondValid(iSecond))
	{
		return -1;
	}
	m_iHour = iHour;
	m_iMinute = iMinute;
	m_iSecond = iSecond;
	return 0;
}
int CDayTime::SetHMS(const char *szTimeString)
{
	if(!szTimeString)
	{
		return -1;
	}
	char *pcTemp = (char *)szTimeString;
	char szToken[32];
	int iHour, iMinute, iSecond;

	pcTemp = GetNextToken(pcTemp, ':', szToken);
	iHour = atoi(szToken);
	pcTemp = GetNextToken(pcTemp, ':', szToken);
	iMinute = atoi(szToken);
	pcTemp = GetNextToken(pcTemp, ':', szToken);
	iSecond = atoi(szToken);
	return SetHMS(iHour, iMinute, iSecond);
}
int CDayTime::IsHourValid(int iHour)
{
	return (iHour >= 0 && iHour < 24);
}
int CDayTime::IsMinuteValid(int iMinute)
{
	return (iMinute >= 0 && iMinute < 60);
}
int CDayTime::IsSecondValid(int iSecond)
{
	return (iSecond >= 0 && iSecond < 60);
}
// 终止: 增加人: 曾宇   日期: 2005-2-25



// 起始 增加人: 曾宇 日期: 2005-6-14
// 修改单号: mainsvr_013_20050610
CTimeVal::CTimeVal()
{
	m_lSec = 0;
	m_lUSec = 0;
}
CTimeVal::CTimeVal(long lSec, long lUSec)
{
	m_lSec = lSec;
	m_lUSec = lUSec;
}
CTimeVal::~CTimeVal()
{
}
long CTimeVal::GetTVSec()
{
	return m_lSec;
}
long CTimeVal::GetTVUSec()
{
	return m_lUSec;
}
CTimeVal& CTimeVal::operator = (const CTimeVal& rtv)
{
	m_lSec = rtv.m_lSec;
	m_lUSec = rtv.m_lUSec;
	return *this;
}
CTimeVal CTimeVal::operator + (const CTimeVal& rtv)
{
	long lSec, lUSec;
	lSec = m_lSec + rtv.m_lSec;
	lUSec = m_lUSec + rtv.m_lUSec;
	lSec += (lUSec/1000000);
	lUSec = lUSec%1000000;
	return CTimeVal(lSec, lUSec);
}
CTimeVal CTimeVal::operator - (const CTimeVal& rtv)
{
	long lSec, lUSec;
	lSec = m_lSec - rtv.m_lSec;
	lUSec = m_lUSec - rtv.m_lUSec;
	if(lUSec < 0)
	{
		lSec--;
		lUSec += 1000000;
	}
	return CTimeVal(lSec, lUSec);
}
CTimeVal CTimeVal::operator / (const int i)
{
	if(i <= 0)
	{
		return CTimeVal(0, 0);
	}
	long lSec, lUSec;
	long long llTmp = m_lSec;
	llTmp *= 1000000;
	llTmp += m_lUSec;
	llTmp /= i;
	lSec = llTmp/1000000;
	lUSec = llTmp%1000000;
	return CTimeVal(lSec, lUSec);
}
int CTimeVal::operator == (const CTimeVal& rtv) const
{
	return (m_lSec == rtv.m_lSec && m_lUSec == rtv.m_lUSec);
}
int CTimeVal::operator != (const CTimeVal& rtv) const
{
	return !(*this == rtv);
}
int CTimeVal::operator > (const CTimeVal& rtv) const
{
	return (m_lSec > rtv.m_lSec || (m_lSec == rtv.m_lSec && m_lUSec > rtv.m_lUSec));
}
int CTimeVal::operator < (const CTimeVal& rtv) const
{
	return !(*this > rtv || *this == rtv);
}
int CTimeVal::operator >= (const CTimeVal& rtv) const
{
	return (*this > rtv || *this == rtv);
}
int CTimeVal::operator <= (const CTimeVal& rtv) const
{
	return (*this < rtv || *this == rtv);
}
// 终止 增加人: 曾宇 日期: 2005-6-14

/*******************************************************************************
Description     : 从时间串中得到时间结构
Input           : struct tm &stTM
: char *strTime 格式为YYYY-MM-DD HH:MM:SS
Output          : 
Return          : int

History         : 
1 Date          : 2005-7-14
Author        : jackyai
Modification  : Created file
*******************************************************************************/
int GetTmFromString( struct tm &stTM, char *strTime)
{
	char *pTime;
	char szTime[32] = {0};

	memset(&stTM, 0, sizeof(stTM));

	strncpy(szTime, strTime, sizeof(szTime) -1);

	if ((NULL == strTime) || (strlen(strTime) < 19)) {
		return 1;
	}

	pTime = &szTime[0];

	//取出年
	pTime[4] = '\0';
	stTM.tm_year = atoi(pTime) - 1900;
	pTime += 5;

	pTime[2] = '\0';
	stTM.tm_mon = atoi(pTime) -1;
	pTime += 3;

	pTime[2] = '\0';
	stTM.tm_mday = atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTM.tm_hour = atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTM.tm_min = atoi(pTime);
	pTime += 3;

	pTime[2] = '\0';
	stTM.tm_sec = atoi(pTime);
	pTime += 3;	

	return 0;
}


int GetDate(time_t tTime,char *sDate)
{
	struct tm   * stru_tm;        

	if (NULL == sDate)
	{
		return -1;
	}

	stru_tm = localtime(&tTime);
	if (stru_tm == NULL)
	{
		*sDate = '\0';
		return -1;
	}
	//sDate[0]='\0';
	sprintf( sDate, "%04d-%02d-%02d", stru_tm->tm_year+1900,
		stru_tm->tm_mon + 1, stru_tm->tm_mday);

	return 0;
}

int GetCurDateTimeString(char *szDateTime)
{
	struct tm *pTempTm = NULL;
	time_t timer;

	if( !szDateTime )
	{
		return -1;
	}

	time( &timer );

	szDateTime[0] = '\0';

	pTempTm = localtime( &timer );

	if( !pTempTm )
	{
		return -1;
	}

	sprintf(szDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
		pTempTm->tm_year + 1900, pTempTm->tm_mon + 1, pTempTm->tm_mday,
		pTempTm->tm_hour, pTempTm->tm_min, pTempTm->tm_sec);

	return 0;
}
