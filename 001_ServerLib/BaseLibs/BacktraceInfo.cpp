/**
*@file BacktraceInfo.cpp
*@author jasonxiong
*@date 2009-11-30
*@version 1.0
*@brief BacktraceInfo的实现文件
*
*
*/

#include <execinfo.h>
#include <string.h>

#include "BacktraceInfo.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

CBacktraceInfo::CBacktraceInfo()
{
    int i;
    for(i = 0; i < MAX_BACKTRACE_SYMBOLS_NUMBER; ++i)
    {
        m_apBacktraceSymbols[i] = NULL;
    }

    m_apszBacktraceInfo = NULL;
    m_iCurTraceSymbolsNum = 0;
    m_szAllBackTraceStr[0] = '\0';
}

CBacktraceInfo::~CBacktraceInfo()
{
    if(m_apszBacktraceInfo)
    {
        free(m_apszBacktraceInfo);
        m_apszBacktraceInfo = NULL;
    }
}

int CBacktraceInfo::GenBacktraceInfo()
{
    if(m_apszBacktraceInfo)
    {
        free(m_apszBacktraceInfo);
        m_apszBacktraceInfo = NULL;
    }

    m_iCurTraceSymbolsNum = backtrace(m_apBacktraceSymbols, MAX_BACKTRACE_SYMBOLS_NUMBER);

    if(m_iCurTraceSymbolsNum > 0)
    {
        m_apszBacktraceInfo = backtrace_symbols(m_apBacktraceSymbols, m_iCurTraceSymbolsNum);

        if(m_apszBacktraceInfo == NULL)
        {
            m_iCurTraceSymbolsNum = 0;
        }
    }

    return 0;
}

const char* CBacktraceInfo::GetAllBackTraceInfo()
{
    if(m_iCurTraceSymbolsNum <= 0 || m_apszBacktraceInfo == NULL)
    {
        return "NO BACKTRACE.";
    }

    SAFE_STRCPY(m_szAllBackTraceStr, "BACKTRACE: \n", sizeof(m_szAllBackTraceStr)-1);
    char szTemp[1024];

    int i;
    for(i = 1; i < m_iCurTraceSymbolsNum; ++i)
    {
        SAFE_SPRINTF(szTemp, sizeof(szTemp)-1, "#%d %s\n", i-1, m_apszBacktraceInfo[i]);
        strncat(m_szAllBackTraceStr, szTemp, sizeof(m_szAllBackTraceStr)-1);
    }

    return m_szAllBackTraceStr;
}

const char* CBacktraceInfo::GetBackTraceInfo(int iTraceDepth)
{
    if(m_iCurTraceSymbolsNum <= 0)
    {
        return "NO BACKTRACE.";
    }

    if(iTraceDepth < 0 || iTraceDepth + 1 >= m_iCurTraceSymbolsNum)
    {
        return "INVALID TRACEDEPTH";
    }

    return m_apszBacktraceInfo[iTraceDepth+1]; //要跳过自己这个函数的堆栈信息所以加1
}
