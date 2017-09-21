#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "CodeEngine.hpp"
#include "oi_tea.hpp"

int EncodeChar( char **pstrEncode, unsigned char ucSrc )
{
    if( pstrEncode == NULL || *pstrEncode == NULL )
    {
        return 0;
    }

    **pstrEncode = (char)ucSrc;
    (*pstrEncode)++;

    return (int)sizeof(unsigned char);
}

int DecodeChar( char **pstrDecode, unsigned char *pucDest )
{
    if( pstrDecode == NULL || *pstrDecode == NULL || pucDest == NULL)
    {
        return 0;
    }

    *pucDest = **pstrDecode;
    (*pstrDecode)++;

    return (int)sizeof(unsigned char);
}

int EncodeShort16( char **pstrEncode, unsigned short usSrc )
{
    //unsigned short usTemp;

    if( pstrEncode == NULL || *pstrEncode == NULL )
    {
        return 0;
    }

    **pstrEncode = (char)((usSrc & 0xFF00 )>> 8);
    (*pstrEncode)++;
    **pstrEncode = (char)(usSrc & 0xFF);
    (*pstrEncode)++;

    return 2;
}

int DecodeShort16( char **pstrDecode, unsigned short *pusDest )
{
    //unsigned short usTemp;

    if( pstrDecode == NULL || *pstrDecode == NULL || pusDest == NULL)
    {
        return 0;
    }

    *pusDest = (unsigned char)**pstrDecode;
    *pusDest <<= 8;
    (*pstrDecode)++;
    *pusDest += (unsigned char)**pstrDecode;
    (*pstrDecode)++;


    return 2;
}

int EncodeLong( char **pstrEncode, unsigned long ulSrc )
{
    unsigned long ulTemp;

    if( pstrEncode == NULL || *pstrEncode == NULL )
    {
        return 0;
    }

    ulTemp = htonl( ulSrc );

    memcpy((void *)(*pstrEncode), (const void *)&ulTemp, sizeof(unsigned long));
    *pstrEncode += sizeof(unsigned long);

    return (int)sizeof(unsigned long);
}


int DecodeLong( char **pstrDecode, unsigned long *pulDest )
{
    unsigned long ulTemp;

    if( pstrDecode == NULL || *pstrDecode == NULL || pulDest == NULL)
    {
        return 0;
    }

    memcpy((void *)&ulTemp, (const void *)(*pstrDecode), sizeof(unsigned long) );
    *pstrDecode += sizeof(unsigned long);

    *pulDest = ntohl(ulTemp);

    return (int)sizeof(unsigned long);
}

int EncodeInt32( char **pstrEncode, unsigned int uiSrc )
{
    //unsigned int uiTemp;

    if( pstrEncode == NULL || *pstrEncode == NULL )
    {
        return 0;
    }

    **pstrEncode = (char)((uiSrc & 0xFF000000 )>> 24);
    (*pstrEncode)++;
    **pstrEncode = (char)((uiSrc & 0xFF0000 )>> 16);
    (*pstrEncode)++;
    **pstrEncode = (char)((uiSrc & 0xFF00 )>> 8);
    (*pstrEncode)++;
    **pstrEncode = (char)(uiSrc & 0xFF);
    (*pstrEncode)++;

    return 4;
}

int DecodeInt32( char **pstrDecode, unsigned int *puiDest )
{
    //unsigned int uiTemp;

    if( pstrDecode == NULL || *pstrDecode == NULL || puiDest == NULL)
    {
        return 0;
    }

    *puiDest = (unsigned char)**pstrDecode;
    *puiDest <<= 8;
    (*pstrDecode)++;

    *puiDest += (unsigned char)**pstrDecode;
    *puiDest <<= 8;
    (*pstrDecode)++;

    *puiDest += (unsigned char)**pstrDecode;
    *puiDest <<= 8;
    (*pstrDecode)++;

    *puiDest += (unsigned char)**pstrDecode;
    (*pstrDecode)++;

    return 4;
}

int EncodeString( char **pstrEncode, char *strSrc, short sMaxStrLength )
{
    //unsigned short usTemp;
    unsigned short ushTempLength;

    if( pstrEncode == NULL || *pstrEncode == NULL || strSrc == NULL || sMaxStrLength <= 0 )
    {
        return 0;
    }

    ushTempLength = (unsigned short)strlen(strSrc);
    if( ushTempLength > sMaxStrLength )
    {
        ushTempLength = sMaxStrLength;
    }

    **pstrEncode = (char)((ushTempLength & 0xFF00 )>> 8);
    (*pstrEncode)++;
    **pstrEncode = (char)(ushTempLength & 0xFF);
    (*pstrEncode)++;

    strncpy((*pstrEncode), strSrc, ushTempLength);

    *pstrEncode += ushTempLength;

    return (ushTempLength + 2);
}


int DecodeString( char **pstrDecode, char *strDest, short sMaxStrLength )
{
    //unsigned short usTemp;
    unsigned short ushTempLength;
    unsigned short usRealLength;

    if( pstrDecode == NULL || *pstrDecode == NULL || strDest == NULL || sMaxStrLength <= 0 )
    {
        return 0;
    }

    ushTempLength = (unsigned char)**pstrDecode;
    ushTempLength <<= 8;
    (*pstrDecode)++;
    ushTempLength += (unsigned char)**pstrDecode;
    (*pstrDecode)++;
    if( ushTempLength > sMaxStrLength )
    {
        usRealLength = sMaxStrLength;
    }
    else
    {
        usRealLength = ushTempLength;
    }
    strncpy(strDest, (*pstrDecode), usRealLength);
    *pstrDecode += ushTempLength;

    strDest[usRealLength] = '\0';
    return (ushTempLength + 2);
}

int EncodeMem( char **pstrEncode, char *pcSrc, short sMemSize )
{
    if( pstrEncode == NULL || *pstrEncode == NULL || pcSrc == NULL || sMemSize <= 0 )
    {
        return 0;
    }

    memcpy((void *)(*pstrEncode), (const void *)pcSrc, sMemSize);
    *pstrEncode += sMemSize;

    return sMemSize;
}

int DecodeMem( char **pstrDecode, char *pcDest, short sMemSize )
{
    if( pstrDecode == NULL || *pstrDecode == NULL || pcDest == NULL || sMemSize <= 0 )
    {
        return 0;
    }

    memcpy((void *)pcDest, (const void *)(*pstrDecode), sMemSize);
    *pstrDecode += sMemSize;

    return sMemSize;
}


int EncodeMemInt( char **pstrEncode, char *pcSrc, int sMemSize )
{
//  unsigned int uiTemp;

    if( pstrEncode == NULL || *pstrEncode == NULL || pcSrc == NULL || sMemSize <= 0 )
    {
        return 0;
    }

    memcpy((void *)(*pstrEncode), (const void *)pcSrc, sMemSize);
    *pstrEncode += sMemSize;

    return sMemSize;
}

int DecodeMemInt( char **pstrDecode, char *pcDest, int sMemSize )
{
//  unsigned int uiTemp;

    if( pstrDecode == NULL || *pstrDecode == NULL || pcDest == NULL || sMemSize <= 0 )
    {
        return 0;
    }

    memcpy((void *)pcDest, (const void *)(*pstrDecode), sMemSize);
    *pstrDecode += sMemSize;

    return sMemSize;
}

int EncodeInt64( char **pstrEncode, uint64_t uiSrc )
{
    uint32_t uiTemp;
    uint32_t uiPartSrc = 0;

    if( pstrEncode == NULL || *pstrEncode == NULL )
    {
        return 0;
    }

    //先编高位的四个字节
    uiPartSrc = (uint32_t)((uiSrc>>32) & 0xFFFFFFFF);
    uiTemp = htonl( uiPartSrc );
    memcpy((void *)(*pstrEncode), (const void *)&uiTemp, sizeof(uint32_t));
    *pstrEncode += sizeof(uint32_t);

    //再编低位的四个字节
    uiPartSrc = (uint32_t)(uiSrc & 0xFFFFFFFF);
    uiTemp = htonl( uiPartSrc );
    memcpy((void *)(*pstrEncode), (const void *)&uiTemp, sizeof(uint32_t));
    *pstrEncode += sizeof(uint32_t);


    return (int)sizeof(uint32_t)*2;
}

int DecodeInt64( char **pstrDecode, uint64_t *puiDest )
{
    uint32_t uiTemp;
    uint64_t ui64Temp = 0;

    if( pstrDecode == NULL || *pstrDecode == NULL || puiDest == NULL)
    {
        return 0;
    }

    memcpy((void *)&uiTemp, (const void *)(*pstrDecode), sizeof(uint32_t) );
    *pstrDecode += sizeof(uint32_t);
    ui64Temp = ntohl(uiTemp);
    ui64Temp <<= 32;

    memcpy((void *)&uiTemp, (const void *)(*pstrDecode), sizeof(uint32_t) );
    *pstrDecode += sizeof(uint32_t);
    ui64Temp += ntohl(uiTemp);

    *puiDest = ui64Temp;

    return (int)sizeof(uint32_t) * 2;
}


int /*CCodeEngine::*/DecryptData(unsigned short ushAlgorithm,
                             const unsigned char* pbyKey,
                             const unsigned char* pbyIn,
                             int iInLength,
                             unsigned char* pbyOut,
                             int& riOutLength)
{
    if( !pbyKey || !pbyIn || !pbyOut || iInLength <= 0 || riOutLength <= 0 )
    {
        return -1;
    }

    switch( ushAlgorithm )
    {
    case 0:
        {
            if( riOutLength < iInLength )
            {
                return -1;
            }

            riOutLength = iInLength;
            memcpy((void *)pbyOut, (const void *)pbyIn, (size_t)iInLength);
            return 0;
        }
    case 1:
        {
            char cTempRet;
            int iTempInLen = 0, iTempOutLen = 0;

            iTempInLen = iInLength;
            iTempOutLen = riOutLength;
            if( iTempOutLen < iTempInLen )
            {
                return -2;
            }

            cTempRet = oi_symmetry_decrypt2((const BYTE *)pbyIn, iTempInLen, (const BYTE *)pbyKey, (BYTE *)pbyOut, &iTempOutLen);

            if( cTempRet )
            {
                riOutLength = iTempOutLen;
                return 0;
            }
            else
            {
                riOutLength = 0;
                return -3;
            }
        }
    case 2:
    case 3:
    default:
        {
            return -4;
        }
    }

    return 0;
}

int /*CCodeEngine::*/Compress(unsigned short ushAlgorithm,
                          const unsigned char* pbyIn,
                          short shInLength,
                          unsigned char* pbyOut,
                          short* pnOutLength)
{
    if( !pbyIn || !pbyOut || !pnOutLength || shInLength <= 0 || *pnOutLength <= 0 )
    {
        return -1;
    }

    switch( ushAlgorithm )
    {
    case 0:
    case 1:
        {
            if( *pnOutLength < shInLength )
            {
                return -1;
            }

            *pnOutLength = shInLength;
            memcpy((void *)pbyOut, (const void *)pbyIn, (size_t)shInLength);

            return 0;
        }
    case 2:
    case 3:
    default:
        {
            return -4;
        }
    }

    return 0;
}

int /*CCodeEngine::*/Uncompress(unsigned short ushAlgorithm,
                            const unsigned char* pbyIn,
                            short shInLength,
                            unsigned char* pbyOut,
                            short* pnOutLength)
{
    if( !pbyIn || !pbyOut || !pnOutLength || shInLength <= 0 || *pnOutLength <= 0 )
    {
        return -1;
    }

    switch( ushAlgorithm )
    {
    case 0:
        {
            if( *pnOutLength < shInLength )
            {
                return -1;
            }

            *pnOutLength = shInLength;
            memcpy((void *)pbyOut, (const void *)pbyIn, (size_t)shInLength);

            return 0;
        }
    case 1:
    case 2:
    case 3:
    default:
        {
            return -4;
        }
    }

    return 0;
}


