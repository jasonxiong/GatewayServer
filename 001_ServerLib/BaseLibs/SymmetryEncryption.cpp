/**
*@file SymmetryEncryption.cpp
*@author jasonxiong
*@date 2009-12-04
*@version 1.0
*@brief 对称加密类实现文件
*
*
*/

#include "SymmetryEncryption.hpp"
#include "oi_tea.hpp"

using namespace ServerLib;

int CSymmetryEncryption::EncryptData(short nAlgorithm, const unsigned char *pbyKey,
                                     const unsigned char *pbyIn, short nInLength,
                                     unsigned char *pbyOut, short *pnOutLength)
{
    if( !pbyKey || !pbyIn || !pbyOut || !pnOutLength || nInLength <= 0 || *pnOutLength <= 0 )
    {
        return -1;
    }

    switch( nAlgorithm )
    {
    case ESEA_NO_ENCRYPT:
    {
        if(*pnOutLength < nInLength )
        {
            return -1;
        }

        *pnOutLength = nInLength;
        memcpy((void *)pbyOut, (const void *)pbyIn, (size_t)nInLength);

        break;
    }

    case ESEA_OI_SYMMETRY_ENCRYPT:
    {
        int iTempInLen = 0, iTempOutLen = 0;

        iTempInLen = nInLength;
        iTempOutLen = oi_symmetry_encrypt2_len(iTempInLen);
        if( *pnOutLength < iTempOutLen )
        {
            return -2;
        }
        oi_symmetry_encrypt2((const unsigned char *)pbyIn, iTempInLen, (const unsigned char *)pbyKey, (unsigned char *)pbyOut, &iTempOutLen);
        *pnOutLength = iTempOutLen;

        break;
    }

    default:
    {
        return -3;
    }
    }

    return 0;
}

int CSymmetryEncryption::DecryptData(short nAlgorithm, const unsigned char *pbyKey,
                                     const unsigned char *pbyIn, short nInLength,
                                     unsigned char *pbyOut, short *pnOutLength)
{
    if( !pbyKey || !pbyIn || !pbyOut || !pnOutLength || nInLength <= 0 || *pnOutLength <= 0 )
    {
        return -1;
    }

    switch( nAlgorithm )
    {
    case ESEA_NO_ENCRYPT:
    {
        if( *pnOutLength < nInLength )
        {
            return -1;
        }

        *pnOutLength = nInLength;
        memcpy((void *)pbyOut, (const void *)pbyIn, (size_t)nInLength);

        break;
    }

    case ESEA_OI_SYMMETRY_ENCRYPT:
    {
        char cTempRet;
        int iTempInLen = 0, iTempOutLen = 0;

        iTempInLen = nInLength;
        iTempOutLen = *pnOutLength;
        if( iTempOutLen < iTempInLen )
        {
            return -2;
        }

        cTempRet = oi_symmetry_decrypt2((const unsigned char *)pbyIn, iTempInLen, (const unsigned char *)pbyKey, (unsigned char *)pbyOut, &iTempOutLen);

        if( cTempRet )
        {
            *pnOutLength = iTempOutLen;
        }
        else
        {
            *pnOutLength = 0;
            return -3;
        }

        break;
    }

    default:
    {
        return -4;
    }
    }

    return 0;
}

