/**
*@file StringUtility.cpp
*@author jasonxiong
*@date
*@version 1.0
*@brief
*
*
*/

#include <string.h>
#include <stdio.h>
#include "StringUtility.hpp"
#include <errno.h>
#include <iconv.h>
#include "LogAdapter.hpp"

using namespace ServerLib;

bool CStringUtility::IsBlankChar(char c)
{
    if(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0')
    {
        return true;
    }

    return false;
}

int CStringUtility::TrimString(char* szTrimString)
{
    if(szTrimString == NULL)
    {
        return -1;
    }

    int iTempLength = strlen(szTrimString);

    if(iTempLength == 0)
    {
        return -2;
    }

    char* pcBeg = szTrimString;

    while(IsBlankChar(*pcBeg) && *pcBeg != '\0')
    {
        ++pcBeg;
    }

    char* pcEnd = &szTrimString[iTempLength-1];

    while (pcEnd >= pcBeg && IsBlankChar(*pcEnd))
    {
        --pcEnd;
    }

    *(pcEnd+1) = '\0';

    strcpy(szTrimString, pcBeg);

    return 0;
}

int CStringUtility::ConvertCode(char* szInBuffer, size_t uiInLength, char* szOutBuffer, size_t uiOutLength, const char* szToCode, const char* szFromCode)
{
    iconv_t handle;

    char* pOriginOutBuffer = szOutBuffer;

    char **pIn = &szInBuffer;

    char **pOut= &szOutBuffer;

    size_t uiTotalOutLength = uiOutLength;
    memset(szOutBuffer, 0, uiTotalOutLength);

    //handle = iconv_open("utf-8", "gb2312");
    handle = iconv_open(szToCode, szFromCode);

    if( handle < 0 )
    {
        return -1;
    }

    if( iconv(handle, pIn, (size_t *)&uiInLength, pOut, (size_t *)&uiOutLength) == (size_t)-1 )
    {
        //fprintf(stderr, "ConvertToCode: Errno = %d, %s\n", errno, strerror(errno));
        TRACESVR("ConvertToCode: Errno = %d, %s\n", errno, strerror(errno));
        iconv_close(handle);
        pOriginOutBuffer[uiTotalOutLength - 1] = '\0';
        return -2;
    }

    iconv_close(handle);
    pOriginOutBuffer[uiTotalOutLength - 1] = '\0';

    return 0;
}
