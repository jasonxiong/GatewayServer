#include "ObjStatistic.hpp"
#include "LogFile.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;


const char* ServerLib::DEFAULT_OBJECT_STAT_DIRECTORY_PATH = "../log/stat/";
const char* ServerLib::DEFAULT_OBJECT_STAT_FILE_NAME = "s";
char ServerLib::g_aszObjectName[ESOTI_OBJECT_ITEM_RESERVER_MAX][MAX_STAT_OBJECT_NAME_LENGTH] = {""};
const char* ServerLib::g_apszObjectItemName[ESOII_OBJECT_ITEM_RESERVER_MAX] = {"Number"};

CObjStatistic::CObjStatistic(void)
{
    m_shObjectTypeNum = 0;
    memset(m_astObjectTypeInfo, 0, sizeof(m_astObjectTypeInfo));
}

CObjStatistic::~CObjStatistic(void)
{
}

int CObjStatistic::Initialize(const char* pszStatPath /* = NULL */, const char* pszStatFileName /* = NULL */)
{
    m_shObjectTypeNum = 0;
    memset(m_astObjectTypeInfo, 0, sizeof(m_astObjectTypeInfo));

    TLogConfig stLogConfig;

    if(pszStatPath)
    {
        SAFE_STRCPY(stLogConfig.m_szPath, pszStatPath, sizeof(stLogConfig.m_szPath)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szPath, DEFAULT_OBJECT_STAT_DIRECTORY_PATH, sizeof(stLogConfig.m_szPath)-1);
    }

    if(pszStatFileName)
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, pszStatFileName, sizeof(stLogConfig.m_szBaseName)-1);
    }
    else
    {
        SAFE_STRCPY(stLogConfig.m_szBaseName, DEFAULT_OBJECT_STAT_FILE_NAME, sizeof(stLogConfig.m_szBaseName)-1);
    }

    SAFE_STRCPY(stLogConfig.m_szExtName, ".stat", sizeof(stLogConfig.m_szExtName)-1);
    stLogConfig.m_iAddDateSuffixType = EADST_DATE_YMD;
    stLogConfig.m_iLogLevel = LOG_LEVEL_ANY;
    stLogConfig.m_shLogType = ELT_LOG_AND_FFLUSH;
    stLogConfig.m_iChkSizeSteps = DEFAULT_CHECK_SIZE_STEPS;
    stLogConfig.m_iMaxFileSize = DEFAULT_MAX_FILE_SIZE;
    stLogConfig.m_iMaxFileCount = DEFAULT_MAX_FILE_COUNT;

    int iRet = m_stObjectStatistic.Initialize("ObjectStat", ESOTI_OBJECT_ITEM_RESERVER_MAX, stLogConfig);

    if(iRet)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);

        return -1;
    }

    return 0;
}

TObjectStatInfo* CObjStatistic::GetObjectStatInfo(int iObjectType)
{
    int i = 0;
    for (i = 0; i < m_shObjectTypeNum; i++)
    {
        if (m_astObjectTypeInfo[i].m_iObjectType == iObjectType)
        {
            return &m_astObjectTypeInfo[i];
        }
    }

    return NULL;
}

int CObjStatistic::AddObjectStatInfo(int iObjectType)
{
    if(m_shObjectTypeNum >= ESOTI_OBJECT_ITEM_RESERVER_MAX)
    {
        SetErrorNO(EEN_MSG_STATISTIC__MSG_ID_REACH_UPPER_LIMIT);

        return -1;
    }

    char szSectionName[MAX_STAT_SECTION_NAME_LENGTH];
    SAFE_STRCPY(szSectionName, g_aszObjectName[iObjectType], sizeof(szSectionName)-1);
    szSectionName[sizeof(szSectionName)-1] = '\0';
    int iSectionIndex = 0;

    int iRet = m_stObjectStatistic.AddSection(szSectionName, iSectionIndex);

    if(iRet < 0)
    {
        SetErrorNO(EEN_MSG_STATISTIC__ADD_SECTION_FAILED);
        return -2;
    }

    m_astObjectTypeInfo[m_shObjectTypeNum].m_iObjectType = iObjectType;
    m_astObjectTypeInfo[m_shObjectTypeNum].m_iObjectIndex = iSectionIndex;

    TStatSection* pstStatSection = m_stObjectStatistic.GetSection(m_astObjectTypeInfo[m_shObjectTypeNum].m_iObjectIndex);
    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -3;
    }

    pstStatSection->m_shStatItemNum = ESOII_OBJECT_ITEM_RESERVER_MAX;

    //初始化统计项
    int i = 0;
    for (i = 0; i < ESOII_OBJECT_ITEM_RESERVER_MAX; i++)
    {
        SAFE_STRCPY(pstStatSection->m_astStatItem[i].m_szName,
                    g_apszObjectItemName[i], sizeof(pstStatSection->m_astStatItem[i].m_szName)-1);
    }

    ++m_shObjectTypeNum;

    return 0;
}

int CObjStatistic::AddObjectStat(int iObjectType, int iObjectNumber)
{
    if (iObjectType < 0  || iObjectType >= ESOTI_OBJECT_ITEM_RESERVER_MAX)
    {
        return -1;
    }

    TObjectStatInfo* pstObjectTypeInfo = GetObjectStatInfo(iObjectType);
    if(pstObjectTypeInfo == NULL)
    {
        int iRet = AddObjectStatInfo(iObjectType);
        if(iRet < 0)
        {
            return -2;
        }

        pstObjectTypeInfo = GetObjectStatInfo(iObjectType);

        //仍然找不到则直接返回失败
        if(pstObjectTypeInfo == NULL)
        {
            SetErrorNO(EEN_MSG_STATISTIC__NULL_POINTER);

            return -3;
        }
    }

    TStatSection* pstStatSection = m_stObjectStatistic.GetSection(pstObjectTypeInfo->m_iObjectIndex);
    if(pstStatSection == NULL)
    {
        SetErrorNO(EEN_MSG_STATISTIC__GET_SECTION_FAILED);

        return -4;
    }

    pstStatSection->m_astStatItem[ESOII_OBJECT_ITEM_NUMBER].m_dValue = iObjectNumber;

    return 0;
}

void CObjStatistic::Print()
{
    m_stObjectStatistic.Print();
}

void CObjStatistic::Reset()
{
    m_stObjectStatistic.Reset();
}

void CObjStatistic::SetObjectStatName(const char** apszObjectName, const int iObjectNumber)
{
    if (!apszObjectName)
    {
        return ;
    }

    if (iObjectNumber >= ESOTI_OBJECT_ITEM_RESERVER_MAX)
    {
        return ;
    }

    int i = 0;
    for (i = 0; i < iObjectNumber; i++)
    {
        if (!apszObjectName[i])
        {
            return ;
        }

        SAFE_STRCPY(g_aszObjectName[i], apszObjectName[i], sizeof(g_aszObjectName[i]) -1);
    }
}

int CObjStatistic::ReloadLogConfig(TLogConfig& rstLogConfig)
{
    m_shObjectTypeNum = 0;
    memset(m_astObjectTypeInfo, 0, sizeof(m_astObjectTypeInfo));

    int iRet = m_stObjectStatistic.Initialize("ObjectStat", ESOTI_OBJECT_ITEM_RESERVER_MAX, rstLogConfig);

    if(iRet < 0)
    {
        SetErrorNO(EEN_MSG_STATISTIC__STATISTIC_INIT_FAILED);
        return -1;
    }

    return 0;
}


