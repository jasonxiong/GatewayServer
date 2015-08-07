/**
*@file DecodeStream.cpp
*@author jasonxiong
*@date 2009-11-30
*@version 1.0
*@brief CDecodeStream的实现文件
*
*
*/

#include <string.h>

#include "DecodeStream.hpp"
#include "ErrorDef.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

CDecodeStream::CDecodeStream()
{
    m_aucCodeBuf = NULL;
    m_pCurCodePos = NULL;
    m_iCodeLen = 0;
    m_iErrorNO = 0;
}

CDecodeStream::~CDecodeStream()
{
    m_aucCodeBuf = NULL;
    m_pCurCodePos = NULL;
    m_iCodeLen = 0;
}

int CDecodeStream::Initialize(unsigned char* pucCodeBuf, int iCodeLen)
{
    if(pucCodeBuf == NULL || iCodeLen <= 0)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return -1;
    }

    m_aucCodeBuf = pucCodeBuf;
    m_pCurCodePos = pucCodeBuf;
    m_iCodeLen = iCodeLen;

    return 0;
}

//解码char
CDecodeStream& CDecodeStream::operator >>(char& rcChar)
{
    return (*this >> (unsigned char&)rcChar);
}

//解码char
CDecodeStream& CDecodeStream::operator >>(unsigned char& rucChar)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(unsigned char) > m_aucCodeBuf + m_iCodeLen)
    {
        SetErrorNO(EEN_DECODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    rucChar = *m_pCurCodePos++;

    return *this;

}

CDecodeStream& CDecodeStream::operator >>(short& rshShort16)
{
    return (*this >> (unsigned short&)rshShort16);
}

//解码short16
CDecodeStream& CDecodeStream::operator >>(unsigned short& rushShort16)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(unsigned short) > m_aucCodeBuf + m_iCodeLen)
    {
        SetErrorNO(EEN_DECODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    rushShort16 = (unsigned char)*m_pCurCodePos++;
    rushShort16 <<= 8;
    rushShort16 += (unsigned char)*m_pCurCodePos++;

    return *this;

}

CDecodeStream& CDecodeStream::operator >>(int& riInt32)
{
    return (*this >> (unsigned int&)riInt32);
}

//解码Int32
CDecodeStream& CDecodeStream::operator >>(unsigned int& ruiInt32)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(unsigned int) > m_aucCodeBuf + m_iCodeLen)
    {
        SetErrorNO(EEN_DECODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    //历史原因，需要调整字节序
    ruiInt32 = (unsigned char)*m_pCurCodePos++;
    ruiInt32 <<= 8;
    ruiInt32 += (unsigned char)*m_pCurCodePos++;
    ruiInt32 <<= 8;
    ruiInt32 += (unsigned char)*m_pCurCodePos++;
    ruiInt32 <<= 8;
    ruiInt32 += (unsigned char)*m_pCurCodePos++;

    return *this;
}

//解码Int64
CDecodeStream& CDecodeStream::operator >>(uint64_t& ruiInt64)
{
    if(m_pCurCodePos == NULL)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return *this;
    }

    if(m_pCurCodePos + sizeof(uint64_t) > m_aucCodeBuf + m_iCodeLen)
    {
        SetErrorNO(EEN_DECODER__EXCEED_CODE_BUF_SIZE);

        return *this;
    }

    unsigned int ui32Temp = 0;

    *this >> ui32Temp;
    ruiInt64 = ui32Temp;
    ruiInt64 <<= 32;
    *this >> ui32Temp;
    ruiInt64 += ui32Temp;

    return *this;
}

//CDecodeStream& CDecodeStream::operator >>(long& rlLong)
//{
//	return (*this >> (unsigned long&)rlLong);
//}
//
////解码long
//CDecodeStream& CDecodeStream::operator >>(unsigned long& rulLong)
//{
//	if(m_pCurCodePos == NULL)
//	{
//		SetErrorNO(EEN_DECODER__NULL_POINTER);
//
//		return *this;
//	}
//
//	if(m_pCurCodePos + sizeof(unsigned long) > m_aucCodeBuf + m_iCodeLen)
//	{
//		SetErrorNO(EEN_DECODER__EXCEED_CODE_BUF_SIZE);
//
//		return *this;
//	}
//
//	memcpy(&rulLong, (const void*)m_pCurCodePos, sizeof(unsigned long));
//	m_pCurCodePos += sizeof(unsigned long);
//
//	return *this;
//
//}

//解码String
int CDecodeStream::DecodeString(char *strDest, short shMaxStrLength)
{
    if(m_pCurCodePos == NULL || strDest == NULL || shMaxStrLength <= 0)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return -1;
    }

    unsigned short ushTempLength = 0;
    ushTempLength = (unsigned char)*m_pCurCodePos++;
    ushTempLength <<= 8;
    ushTempLength += (unsigned char)*m_pCurCodePos++;

    if(m_pCurCodePos + (unsigned int)ushTempLength > m_aucCodeBuf + m_iCodeLen)
    {
        SetErrorNO(EEN_DECODER__EXCEED_CODE_BUF_SIZE);

        return -2;
    }

    unsigned short ushRealLength = 0;

    if( ushTempLength > shMaxStrLength )
    {
        ushRealLength = shMaxStrLength;
    }
    else
    {
        ushRealLength = ushTempLength;
    }

    SAFE_STRCPY(strDest, (char*)m_pCurCodePos, ushRealLength);
    strDest[ushRealLength-1] = '\0';
    m_pCurCodePos += ushTempLength;

    return 0;
}

//解码Mem
int CDecodeStream::DecodeMem(char *pcDest, int iMemorySize)
{
    if(m_pCurCodePos == NULL || pcDest == NULL || iMemorySize <= 0)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return -1;
    }

    if(m_pCurCodePos + (unsigned int)iMemorySize > m_aucCodeBuf + m_iCodeLen)
    {
        SetErrorNO(EEN_DECODER__NULL_POINTER);

        return -2;
    }

    memcpy(pcDest, m_pCurCodePos, iMemorySize);
    m_pCurCodePos += (unsigned int)iMemorySize;

    return 0;
}

