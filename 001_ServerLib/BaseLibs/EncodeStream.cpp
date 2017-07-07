/**
*@file EncodeStream.cpp
*@author jasonxiong
*@date 2009-11-30
*@version 1.0
*@brief CEncodeStream的实现文件
*
*
*/

#include <string.h>

#include "EncodeStream.hpp"
#include "ErrorDef.hpp"
#include "StringUtility.hpp"

using namespace ServerLib;

CEncodeStream::CEncodeStream()
{
    m_aucCodeBuf = NULL;
    m_pCurCodePos = NULL;
    m_iMaxCodeLen = 0;
    m_iErrorNO = 0;
}

CEncodeStream::~CEncodeStream()
{
    m_aucCodeBuf = NULL;
    m_pCurCodePos = NULL;
    m_iMaxCodeLen = 0;
}

int CEncodeStream::Initialize(unsigned char* pucCodeBuf, int iMaxCodeLen)
{
    if(pucCodeBuf == NULL || iMaxCodeLen <= 0)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);

        return -1;
    }

    m_aucCodeBuf = pucCodeBuf;
    m_pCurCodePos = pucCodeBuf;
    m_iMaxCodeLen = iMaxCodeLen;

    return 0;
}


CEncodeStream& CEncodeStream::operator <<(char cChar)
{
    return (*this << (unsigned char)cChar);
}

CEncodeStream& CEncodeStream::operator <<(unsigned char ucChar)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(unsigned char) > m_aucCodeBuf + m_iMaxCodeLen)
    {
        SetErrorNO(EEN_ENCODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    *m_pCurCodePos++ = ucChar;

    return *this;
}

CEncodeStream& CEncodeStream::operator <<(short shShort16)
{
    return (*this << (unsigned short)shShort16);
}

CEncodeStream& CEncodeStream::operator <<(unsigned short ushShort16)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(unsigned short) > m_aucCodeBuf + m_iMaxCodeLen)
    {
        SetErrorNO(EEN_ENCODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    *m_pCurCodePos++ = (char)((ushShort16 & 0xFF00) >> 8);
    *m_pCurCodePos++ = (char)(ushShort16 & 0xFF);

    return *this;
}

CEncodeStream& CEncodeStream::operator <<(int iInt32)
{
    return (*this << (unsigned int)iInt32);
}

CEncodeStream& CEncodeStream::operator <<(unsigned int uiInt32)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);
        return *this;
    }

    if(m_pCurCodePos + sizeof(unsigned int) > m_aucCodeBuf + m_iMaxCodeLen)
    {
        SetErrorNO(EEN_ENCODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    //历史原因，需要调整字节序
    *m_pCurCodePos++ = (char)((uiInt32 & 0xFF000000) >> 24);
    *m_pCurCodePos++ = (char)((uiInt32 & 0xFF0000) >> 16);
    *m_pCurCodePos++ = (char)((uiInt32 & 0xFF00) >> 8);
    *m_pCurCodePos++ = (char)(uiInt32 & 0xFF);

    return *this;
}

CEncodeStream& CEncodeStream::operator <<(uint64_t uiInt64)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(uint64_t) > m_aucCodeBuf + m_iMaxCodeLen)
    {
        SetErrorNO(EEN_ENCODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    unsigned int ui32Temp;

    ui32Temp = (unsigned int)((uiInt64 >> 32) & 0xFFFFFFFF);
    *this << ui32Temp;

    ui32Temp = (unsigned int)(uiInt64 & 0xFFFFFFFF);
    *this << ui32Temp;

    return *this;
}

//编码String
int CEncodeStream::EncodeString(char* pszString, short shMaxStrLength)
{
    if(m_pCurCodePos == NULL || pszString == NULL || shMaxStrLength <= 0)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);

        return -1;
    }

    //unsigned short usTemp;
    unsigned short ushTempLength = (unsigned short)strlen(pszString);

    if(ushTempLength > (unsigned short)shMaxStrLength)
    {
        ushTempLength = (unsigned short)shMaxStrLength;
    }

    if(m_pCurCodePos + (unsigned int)ushTempLength > m_aucCodeBuf + m_iMaxCodeLen)
    {
        SetErrorNO(EEN_ENCODER__EXCEED_CODE_BUF_SIZE);

        return -2;
    }

    *m_pCurCodePos++ = (char)((ushTempLength & 0xFF00) >> 8);
    *m_pCurCodePos++ = (char)(ushTempLength & 0xFF);
    SAFE_STRCPY((char*)m_pCurCodePos, pszString, ushTempLength);

    //字符串长度达到Buffer最大长度，需要将最后一个字符改为\0
    if(ushTempLength == shMaxStrLength)
    {
        m_pCurCodePos[ushTempLength-1] = '\0';
    }

    m_pCurCodePos += (unsigned int)ushTempLength;

    return 0;
}

//编码Mem
int CEncodeStream::EncodeMem(char *pcSrc, int iMemorySize)
{
    if(m_pCurCodePos == NULL || pcSrc == NULL || iMemorySize <= 0)
    {
        SetErrorNO(EEN_ENCODER__NULL_POINTER);

        return -1;
    }

    if(m_pCurCodePos + (unsigned int)iMemorySize > m_aucCodeBuf + m_iMaxCodeLen)
    {
        SetErrorNO(EEN_ENCODER__EXCEED_CODE_BUF_SIZE);

        return -2;
    }

    memcpy(m_pCurCodePos, pcSrc, iMemorySize);
    m_pCurCodePos += (unsigned int)iMemorySize;

    return 0;
}

