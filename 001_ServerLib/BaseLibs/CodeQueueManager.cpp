#include "CodeQueueManager.hpp"
#include "SectionConfig.hpp"
#include "StringUtility.hpp"
#include "LogAdapter.hpp"

using namespace ServerLib;
int CCodeQueueManager::LoadCodeQueueConfig(const char* szConfigFile, const char* szModuleName)
{
    m_iCodeQueueNum = 0;

    // ∂¡»°≈‰÷√Œƒº˛
    CSectionConfig stConfigFile;
    int iRet = stConfigFile.OpenFile(szConfigFile);
    if (iRet < 0)
    {
        return -1;
    }

    stConfigFile.GetItemValue("CodeQueue", "CodeQueueNum", m_iCodeQueueNum, 1);
    if (m_iCodeQueueNum <= 0 || m_iCodeQueueNum >= MAX_CODE_QUEUE_NUMBER)
    {
        return -10;
    }

    stConfigFile.GetItemValue("CodeQueue", "CodeQueuePath", m_szCodeQueuePath, sizeof(m_szCodeQueuePath), "../../../Common/");

    for (int i = 0; i < m_iCodeQueueNum; i++)
    {
        char sKeyName[255];
        char sDefaultValue[255];
        snprintf(sKeyName, sizeof(sKeyName), "GatewayTo%s_%d",szModuleName, i);
        snprintf(sDefaultValue, sizeof(sDefaultValue), "CQGatewayTo%s_%d.shm", szModuleName, i);
        stConfigFile.GetItemValue("CodeQueue", sKeyName, m_aszLotusToGameCQ[i], sizeof(m_aszLotusToGameCQ[i]), sDefaultValue);

        snprintf(sKeyName, sizeof(sKeyName), "%sToGateway_%d", szModuleName, i);
        snprintf(sDefaultValue, sizeof(sDefaultValue), "CQ%sToGateway_%d.shm", szModuleName, i);
        stConfigFile.GetItemValue("CodeQueue", sKeyName, m_aszGameToLotusCQ[i], sizeof(m_aszGameToLotusCQ[i]), sDefaultValue);
    }

    stConfigFile.CloseFile();

    return 0;
}

int CCodeQueueManager::Initialize(bool bResumeMode)
{
    if (m_iCodeQueueNum <= 0 || m_iCodeQueueNum >= MAX_CODE_QUEUE_NUMBER)
    {
        return -1;
    }

    //init
    memset(m_apLotusCodeQueueIn, 0, sizeof(m_apLotusCodeQueueIn));
    memset(m_apLotusCodeQueueOut, 0, sizeof(m_apLotusCodeQueueOut));

    char szShmPath[256];
    for (int i = 0; i < m_iCodeQueueNum; i++)
    {
        SAFE_SPRINTF(szShmPath, sizeof(szShmPath), "%s%s", m_szCodeQueuePath, m_aszLotusToGameCQ[i]);
        m_apLotusCodeQueueIn[i] = CCodeQueue::CreateBySharedMemory(szShmPath, 'I', 16777216);
        if(!m_apLotusCodeQueueIn[i])
        {
            TRACESVR("CreateObject CodeQueueIn Failed!\n");
            return -2;
        }

        m_apLotusCodeQueueIn[i]->Initialize(bResumeMode);

        SAFE_SPRINTF(szShmPath, sizeof(szShmPath), "%s%s", m_szCodeQueuePath, m_aszGameToLotusCQ[i]);
        m_apLotusCodeQueueOut[i] = CCodeQueue::CreateBySharedMemory(szShmPath, 'O', 16777216);
        if(!m_apLotusCodeQueueOut[i])
        {
            TRACESVR("CreateObject CodeQueueOut Failed!\n");
            return -3;
        }

        m_apLotusCodeQueueOut[i]->Initialize(bResumeMode);
    }

    return 0;
}

int CCodeQueueManager::SendOneMsg(const char* pszMsg, int iMsgLength, int iInstanceID)
{
    if (iInstanceID >= m_iCodeQueueNum)
    {
        return -1;
    }
    if (m_apLotusCodeQueueOut[iInstanceID] == NULL)
    {
        return -2;
    }

    return m_apLotusCodeQueueOut[iInstanceID]->PushOneCode((unsigned char*)pszMsg, iMsgLength);

}
int CCodeQueueManager::RecvOneMsg(char* pszMsg, int iMaxOutMsgLen, int& riMsgLength, int iInstanceID)
{
    if (iInstanceID >= m_iCodeQueueNum)
    {
        return -1;
    }
    if (m_apLotusCodeQueueIn[iInstanceID] == NULL)
    {
        return -2;
    }

    return m_apLotusCodeQueueIn[iInstanceID]->PopOneCode((unsigned char*)pszMsg, iMaxOutMsgLen, riMsgLength);
}
