
#include <errno.h>
#include "base.hpp"
#include "BillWriter.hpp"
#include "TimeTools.hpp"

int CBillWriter::WriteBlockBill(unsigned int uiIP)
{
	FILE *fpBill = NULL;
	char fname[128];
	TStruTime stTempNow;
	char szCurDate[32];
	char szCurDateTime[32];
	char szIP[32];
	char szCurMon[32];
	char szBillDir[64];
	sockaddr_in stSockAddr;
	time_t tNow;

	time(&tNow);
	GetStructTime(tNow, &stTempNow);
	sprintf(szCurMon, "%04d-%02d", stTempNow.m_iYear, stTempNow.m_iMon);
	sprintf(szCurDate, "%04d-%02d-%02d", stTempNow.m_iYear, stTempNow.m_iMon, stTempNow.m_iDay);
	sprintf(szCurDateTime, "%04d%02d%02d%02d%02d%02d", stTempNow.m_iYear, stTempNow.m_iMon, stTempNow.m_iDay, stTempNow.m_iHour, stTempNow.m_iMin, stTempNow.m_iSec);
	sprintf(szBillDir, "%s/%s", "../TCPBill", szCurMon);

	//检查并创建帐单目录
	DIR* dpBill = opendir(szBillDir);
	if(!dpBill)
	{
		if(mkdir(szBillDir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			printf("In %s, can't create bill dir %s, Err: %s.\n", __FUNCTION__, szBillDir, strerror(errno));
			return -1;
		}
	}
	else
	{
		closedir(dpBill);
	}

	sprintf(fname, "%s/bill-%s.txt", szBillDir, szCurDate);
	fpBill = fopen(fname, "a+");
	if(!fpBill)
	{
		printf("Open bill file %s failed， Err: %s\n", fname, strerror(errno));
		return -1;
	}

	stSockAddr.sin_addr.s_addr = uiIP;
	stSockAddr.sin_port = 0;
	memcpy(szIP, inet_ntoa(stSockAddr.sin_addr), sizeof(szIP));

	fprintf(fpBill, "%s|%s|\n", szCurDateTime, szIP);

	fclose(fpBill);

	return 0;
}

int CBillWriter::WriteSocketBill(TExternalClientSocket* pstClientSocket, short shEventID)
{
	if(pstClientSocket == NULL)
	{
		return -1;
	}

	FILE *fpBill = NULL;
	char fname[128];
	TStruTime stTempNow;
	char szCurDate[32];
	char szCurDateTime[32];
	char szIP[32];
	char szCurMon[32];
	char szBillDir[64];
	sockaddr_in stSockAddr;
	time_t tNow;

	time(&tNow);
	GetStructTime(tNow, &stTempNow);
	sprintf(szCurMon, "%04d-%02d", stTempNow.m_iYear, stTempNow.m_iMon);
	sprintf(szCurDate, "%04d-%02d-%02d", stTempNow.m_iYear, stTempNow.m_iMon, stTempNow.m_iDay);
	sprintf(szCurDateTime, "%04d%02d%02d%02d%02d%02d", stTempNow.m_iYear, stTempNow.m_iMon, stTempNow.m_iDay, stTempNow.m_iHour, stTempNow.m_iMin, stTempNow.m_iSec);
	sprintf(szBillDir, "%s/%s", "../TCPBill", szCurMon);

	//检查并创建帐单目录
	DIR* dpBill = opendir(szBillDir);
	if(!dpBill)
	{
		if(mkdir(szBillDir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			printf("In %s, can't create bill dir %s, Err: %s.\n", __FUNCTION__, szBillDir, strerror(errno));
			return -1;
		}
	}
	else
	{
		closedir(dpBill);
	}

	sprintf(fname, "%s/socketbill-%s.txt", szBillDir, szCurDate);
	fpBill = fopen(fname, "a+");
	if(!fpBill)
	{
		printf("Open bill file %s failed， Err: %s\n", fname, strerror(errno));
		return -1;
	}

	stSockAddr.sin_addr.s_addr = pstClientSocket->m_uiSrcIP;
	stSockAddr.sin_port = 0;
	memcpy(szIP, inet_ntoa(stSockAddr.sin_addr), sizeof(szIP));

	//Time|ClientIP|Port|FD|Uin|CreateTime|SendFlag|EventID|
	fprintf(fpBill, "%s|%s|%d|%d|%u|%d|%d|%d|\n", szCurDateTime, szIP, pstClientSocket->m_ushSrcPort, pstClientSocket->m_iSocketFD, 
        pstClientSocket->m_uiUin, pstClientSocket->m_uiCreateTime, pstClientSocket->m_iSendFlag, shEventID);

	fclose(fpBill);

	return 0;
}
