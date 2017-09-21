
#ifndef __TIMETOOLS_HPP__
#define __TIMETOOLS_HPP__

#include <time.h>

#ifdef WIN32
#include <winsock2.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

typedef struct
{
	int m_iYear;
	int m_iMon;
	int m_iDay;
	int m_iHour;
	int m_iMin;
	int m_iSec;
	int m_iMSec;
} TStruTime;

int GetCurDateTimeString(char *szDateTime);
int GetTimeStringOfDay(char *strTime);
int GetDateString(char *strTime);

int GetStructTime(time_t tTime, TStruTime *pstTime);
int GetCurStructTime(TStruTime *pstTime);
int GetTStruTimeFromString(char *strTime, TStruTime &stTime);

int GetTimeString( time_t timer, char *strTime);

int TimeValPlus(timeval& tvA, timeval& tvB, timeval& tvResult);
int TimeValMinus(timeval& tvA, timeval& tvB, timeval& tvResult);

void BeginGetTimeVal();
void EndGetTimeVal(timeval& tvVal);

int GetDate(time_t tTime,char *sDate);
//#ifdef __cplusplus
//}
//#endif


class CDayTime
{
public:
	CDayTime();
	CDayTime(time_t tSecTime);
	CDayTime(int iHour, int iMinute, int iSecond);
	~CDayTime();

	CDayTime& operator = (const CDayTime& rDayTime);
	int operator == (const CDayTime& rDayTime) const;
	int operator != (const CDayTime& rDayTime) const;
	int operator > (const CDayTime& rDayTime) const;
	int operator < (const CDayTime& rDayTime) const;
	int operator >= (const CDayTime& rDayTime) const;
	int operator <= (const CDayTime& rDayTime) const;
	int SetHMS(int iHour, int iMinute, int iSecond);
	int SetHMS(const char *szTimeString);
	void GetHMS(int& riH, int& riM, int &riS);
	void GetString(char *szTString);

private:
	int IsHourValid(int iHour);
	int IsMinuteValid(int iMinute);
	int IsSecondValid(int iSecond);

	int m_iHour:6;
	int m_iMinute:7;
	int m_iSecond:7;
};


class CTimeVal
{
public:
	CTimeVal();
	CTimeVal(long lSec, long lUSec);
	~CTimeVal();

	CTimeVal& operator =(const CTimeVal& rtv);
	CTimeVal operator +(const CTimeVal& rtv);
	CTimeVal operator -(const CTimeVal& rtv);
	CTimeVal operator /(const int i);
	int operator ==(const CTimeVal& rtv) const;
	int operator !=(const CTimeVal& rtv) const;
	int operator >(const CTimeVal& rtv) const;
	int operator <(const CTimeVal& rtv) const;
	int operator >=(const CTimeVal& rtv) const;
	int operator <=(const CTimeVal& rtv) const;
	long GetTVSec();
	long GetTVUSec();

private:
	long m_lSec;
	long m_lUSec;
};
// 终止 增加人: 曾宇 日期: 2005-6-14



#endif

