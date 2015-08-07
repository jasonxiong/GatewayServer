/**
*@file LineConfig.cpp
*@author jasonxiong
*@date 2009-11-19
*@version 1.0
*@brief
*
*
*/

#include <stdio.h>
#include <stdarg.h>

#include "CommonDef.hpp"
#include "LineConfig.hpp"
#include "StringUtility.hpp"

using namespace ServerLib;

CLineConfig::CLineConfig()
{
    m_pfCfgFile = NULL;
    m_bReachEOF = false;
    m_iErrorNO = 0;
}

CLineConfig::~CLineConfig()
{
    CloseFile();
}

void CLineConfig::CloseFile()
{
    if(m_pfCfgFile)
    {
        fclose(m_pfCfgFile);
    }

    m_pfCfgFile = NULL;
}

int CLineConfig::OpenFile(const char *pszFilename)
{
    if(pszFilename == NULL)
    {
        SetErrorNO(EEN_LINE_CONFIG__NULL_POINTER);

        return -1;
    }

    if(m_pfCfgFile)
    {
        CloseFile();
    }

    m_pfCfgFile = fopen(pszFilename, "rb");

    if(m_pfCfgFile == NULL)
    {
        SetErrorNO(EEN_LINE_CONFIG__OPEN_FILE_FAILED);

        return -2;
    }

    m_bReachEOF = false;

    return 0;
}

int CLineConfig::GetItemValue(const char* pcContent, ...)
{
    if(m_pfCfgFile == NULL)
    {
        SetErrorNO(EEN_LINE_CONFIG__NULL_POINTER);

        return -1;
    }

    if(m_bReachEOF)
    {
        SetErrorNO(EEN_LINE_CONFIG__REACH_EOF);

        return -2;
    }

    char szLineBuf[MAX_LINE_BUF_LENGTH];
    szLineBuf[0] = '\0';
    bool bValidLine = false;

    while(!bValidLine)
    {
        fgets(szLineBuf, sizeof(szLineBuf), m_pfCfgFile);

        if(feof(m_pfCfgFile))
        {
            m_bReachEOF = true;

            return 0;
        }

        CStringUtility::TrimString(szLineBuf);

        //是注释的行或者空行，则继续往下读取
        if(szLineBuf[0] == '#' || szLineBuf[0] == '\0')
        {
            bValidLine = false;
        }
        else
        {
            bValidLine = true;
        }
    }


    va_list ap;
    va_start(ap, pcContent);
    int iReadItemNum = vsscanf(szLineBuf, pcContent, ap);
    va_end(ap);

    if(iReadItemNum < 0)
    {
        iReadItemNum = 0;
    }

    return iReadItemNum;
}
