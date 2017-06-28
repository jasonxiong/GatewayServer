#ifndef _BASE_CPP_
#define _BASE_CPP_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>

#include "base.hpp"

#ifndef WIN32
FILE* g_fpOutPut = NULL;
#endif

#define MAX_CONFIG_LINE_LEN 255

void TrimStr( char *strInput )
{
    char *pb;
    char *pe;
    int iTempLength;

    if( strInput == NULL )
    {
        return;
    }

    iTempLength = strlen(strInput);
    if( iTempLength == 0 )
    {
        return;
    }

    pb = strInput;

    while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
    {
        pb ++;
    }

    pe = &strInput[iTempLength-1];
    while ((pe >= pb) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
    {
        pe --;
    }

    *(pe+1) = '\0';

    strcpy( strInput, pb );
    return;
}

char* GetNextToken(const char *szSrc, char cSplitter, char *szToken)
{
    char *pcPos = NULL;
    int iTokenLength = 0;

    if( !szSrc )
    {
        if( szToken )
        {
            szToken[0] = '\0';
        }
        return NULL;
    }

    pcPos = const_cast<char *>(strchr(szSrc, (int)cSplitter));



    if( pcPos )
    {
        if( szToken )
        {
            iTokenLength = (int)(pcPos - szSrc);
            memcpy((void *)szToken, (const void *)szSrc, iTokenLength);
            szToken[iTokenLength] = '\0';
        }
        pcPos++;
    }
    else
    {
        if( szToken )
        {
            strcpy(szToken, szSrc);
        }
    }

    return pcPos;
}

void PrintBin(char *pBuffer, int iLength)
{
    int i;
    char szBuf[30*1024];
    char szTmp[1024];
    if( iLength <= 0 || iLength > 4096 || pBuffer == NULL )
    {
        return;
    }
    szBuf[0] = 0;

    for( i = 0; i < iLength; i++ )
    {
        if( !(i%16) )
        {
            sprintf(szTmp,"\n%04d>    ", i/16+1);
            strcat(szBuf,szTmp);
        }
        sprintf(szTmp,"%02X ", (unsigned char)pBuffer[i]);
        strcat(szBuf,szTmp);
    }
    PRINT("%s",szBuf);
    PRINT("\n");

    return;
}

void xprintf(const char *szFormat, ...)
{
#ifndef WIN32
    va_list ag;

    if( !g_fpOutPut )
    {
        return;
    }

    va_start( ag, szFormat );
    vfprintf(g_fpOutPut, szFormat, ag);
    fflush(g_fpOutPut);
    va_end( ag );
#endif
    return;
}

void nullprintf(const char *szFormat, ...)
{
    return;
}

unsigned int GenRandSeed(int iRandomPara)
{
    struct timeb timebuffer;
    unsigned int uiSeed;

    ftime(&timebuffer);

    uiSeed = (unsigned int)(( timebuffer.time  & 0xFFFF) + timebuffer.millitm + iRandomPara) & (unsigned int)timebuffer.millitm;

    return uiSeed;
}

int CreateDir(const char *szDir)
{
    if (NULL == szDir)
    {
        return -1;
    }

#ifndef WIN32
    DIR* dpLog = opendir( (const char *)szDir );
    if( !dpLog )
    {
        if( mkdir(szDir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) )
        {
            return -2;
        }
    }
    else
    {
        closedir( dpLog );
    }
#endif
    return 0;
}

void HexPrint(void *pBuffer, int iLength, FILE *fpOut /* = stdout */)
{
    int i;

    if(iLength <= 0 || !pBuffer || !fpOut)
    {
        return;
    }
    unsigned char *pTmp = (unsigned char *)pBuffer;
    for(i = 0; i < iLength; i++)
    {
        if( !(i%16) )
        {
            fprintf(fpOut, "\n%04d>    ", i/16+1);
        }
        fprintf(fpOut, "%02X ", pTmp[i]);
    }
    fprintf(fpOut, "\n");
    return;
}

#endif
