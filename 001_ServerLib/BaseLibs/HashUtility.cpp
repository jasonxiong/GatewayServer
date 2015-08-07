
#include "HashUtility.hpp"

//Robert Sedgwicks hash
long CHashUtility::RsHash(const char* pszStr, int iLen)
{
    int iSeed = 63689;
    int iStep = 378551;

    long lHash = 0;

    for(int i=0; i<iLen; ++i)
    {
        lHash = lHash*iSeed + pszStr[i];
        iSeed *= iStep;
    }

    return lHash;
}

//Justin Sobel hash
long CHashUtility::JSHash(const char* pszStr, int iLen)
{
    long lHash = 1315423911;

    for(int i=0; i<iLen; ++i)
    {
        lHash ^= ((lHash<<5) + pszStr[i] + (lHash>>2));
    }

    return lHash;
}

//PJW hash
long CHashUtility::PJWHash(const char* pszStr, int iLen)
{
    long lBitsInUnsignedInt = (long)(4*8);
    long lThreeQuarters = (long)((lBitsInUnsignedInt*3)/4);
    long lOneEighth = (long)(lBitsInUnsignedInt/8);
    long lHighBits = (long)(0xFFFFFFFF) << (lBitsInUnsignedInt - lOneEighth);
    long lHash = 0;
    long lTest = 0;
 
    for(int i=0; i<iLen; i++)  
    {
        lHash = (lHash<<lOneEighth) + pszStr[i];
        if((lTest=lHash&lHighBits) != 0)
        {
            lHash = ((lHash^(lTest>>lThreeQuarters)) & (~lHighBits));
        }
    }  

    return lHash;  
}

//ELF hash, just like PJW
long CHashUtility::ELFHash(const char* pszStr, int iLen)
{
    long lHash = 0;
    long lX = 0;

    for(int i=0; i<iLen; ++i)
    {
        lHash = (lHash<<4) + pszStr[i];
        if((lX=lHash&0xF0000000L) != 0)
        {
            lHash ^= (lX>>24);
        }

        lHash &= ~lX;
    }

    return lHash;
}

//BKDR hash
long CHashUtility::BKDRHash(const char* pszStr, int iLen)
{
    long lSeed = 131;
    long lHash = 0;

    for(int i=0; i<iLen; ++i)
    {
        lHash = (lHash*lSeed) + pszStr[i];
    }

    return lHash; 
}

//SDBM hash
long CHashUtility::SDBMHash(const char* pszStr, int iLen)
{
    long lHash = 0;

    for(int i=0; i<iLen; ++i)
    {
        lHash = pszStr[i] + (lHash<<6) + (lHash<<16) - lHash;
    }

    return lHash;
}

//DJB hash, 目前公认的最有效的hash算法
long CHashUtility::DJBHash(const char* pszStr, int iLen)
{
    long lHash = 5381;
    
    for(int i=0; i<iLen; ++i)
    {
        lHash = ((lHash<<5)+lHash) + pszStr[i];
    }

    return lHash;
}

//DEK hash
long CHashUtility::DEKHash(const char* pszStr, int iLen)
{
    long lHash = iLen;

    for(int i=0; i<iLen; ++i)
    {
        lHash = ((lHash<<5)^(lHash>>27))^pszStr[i];
    }

    return lHash;
}

