#include "LotusLogAdapter.hpp"
#include "CodeQueue.hpp"
#include "CodeQueueAssemble.hpp"
#include "LotusStatistic.hpp"
#include "ShmObjectCreator.hpp"



int CCodeQueueAssemble::Initialize()
{
    m_iCodeQueueNumber = 0;

    //初始化三个特殊输入CodeQueue
    m_pstDefaultInputCodeQueue = 0;
    m_pstRecordInputCodeQueue = 0;

    //初始化三个特殊输出CodeQueue
    m_pstDefaultOutputCodeQueue = 0;
    m_pstRecordOutputCodeQueue = 0;

    return 0;
}

int CCodeQueueAssemble::CreateCodeQueue(const TCodeQueueConfig& rstCodeQueueConfig)
{
    CCodeQueue*& rpstCodeQueue = m_apstCodeQueue[rstCodeQueueConfig.m_iCodeQueueID];

    rpstCodeQueue = CCodeQueue::CreateBySharedMemory(rstCodeQueueConfig.m_szKeyFileName, rstCodeQueueConfig.m_ucKeyPrjID,
        rstCodeQueueConfig.m_uiShmSize);

    TRACESVR(LOG_LEVEL_DETAIL, "CodeQueue Size= %u, %u, %u.\n", rstCodeQueueConfig.m_uiShmSize, sizeof(CCodeQueue), sizeof(CSharedMemory));

    if(!rpstCodeQueue)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CCodeQueueAssemble::CreateCodeQueue rpstCodeQueue=NULL\n");
        return -1;
    }

    rpstCodeQueue->Initialize();

    printf("CodeQueue[%d]:%s is Ready\n", rstCodeQueueConfig.m_iCodeQueueID, rstCodeQueueConfig.m_szKeyFileName);

    if(rstCodeQueueConfig.m_uiCodeQueueFlag & ECDF_OUTPUT)
    {
        //Output
        if(rstCodeQueueConfig.m_uiCodeQueueType == ECQT_DEFAULT)
        {
            m_pstDefaultOutputCodeQueue = rpstCodeQueue;
            TRACESVR(LOG_LEVEL_DETAIL, "In CCodeQueueAssemble::CreateCodeQueue ECQT_DEFAULT ID:%d Flag:%u Type:%u\n",
                rstCodeQueueConfig.m_iCodeQueueID, rstCodeQueueConfig.m_uiCodeQueueFlag, rstCodeQueueConfig.m_uiCodeQueueType);
        }
        else if(rstCodeQueueConfig.m_uiCodeQueueType == ECQT_RECORD)
        {
            m_pstRecordOutputCodeQueue = rpstCodeQueue;
            TRACESVR(LOG_LEVEL_DETAIL, "In CCodeQueueAssemble::CreateCodeQueue ECQT_RECORD ID:%d Flag:%u Type:%u\n",
                rstCodeQueueConfig.m_iCodeQueueID, rstCodeQueueConfig.m_uiCodeQueueFlag, rstCodeQueueConfig.m_uiCodeQueueType);
        }
    }
    else
    {
        //Input
        if(rstCodeQueueConfig.m_uiCodeQueueType == ECQT_DEFAULT)
        {
            m_pstDefaultInputCodeQueue = rpstCodeQueue;
            TRACESVR(LOG_LEVEL_DETAIL, "In CCodeQueueAssemble::CreateCodeQueue ECQT_DEFAULT ID:%d Flag:%u Type:%u\n",
                rstCodeQueueConfig.m_iCodeQueueID, rstCodeQueueConfig.m_uiCodeQueueFlag, rstCodeQueueConfig.m_uiCodeQueueType);
        }
        else if(rstCodeQueueConfig.m_uiCodeQueueType == ECQT_RECORD)
        {
            m_pstRecordInputCodeQueue = rpstCodeQueue;
            TRACESVR(LOG_LEVEL_DETAIL, "In CCodeQueueAssemble::CreateCodeQueue ECQT_RECORD ID:%d Flag:%u Type:%u\n",
                rstCodeQueueConfig.m_iCodeQueueID, rstCodeQueueConfig.m_uiCodeQueueFlag, rstCodeQueueConfig.m_uiCodeQueueType);
        }
    }

    return 0;
}

int CCodeQueueAssemble::PushToDefault(const unsigned short ushCodeLength, const char* pCodeBuffer,
                                      const bool bIsRecordEnabled)
{
    int iRet = 0;

    iRet = m_pstDefaultInputCodeQueue->PushOneCode((const unsigned char*)pCodeBuffer, ushCodeLength);

    if(iRet < 0)
    {
        LotusStatistic::instance()->AddFailedPushNumber(1);
        return iRet;
    }

    if(bIsRecordEnabled)
    {
        PushToRecord(ushCodeLength, pCodeBuffer);
    }

    return 0;
}

int CCodeQueueAssemble::PushToRecord(const unsigned short ushCodeLength, const char* pCodeBuffer)
{
    if(!m_pstRecordInputCodeQueue)
    {
        return -1;
    }

    int iRet = 0;

    iRet = m_pstRecordInputCodeQueue->PushOneCode((const unsigned char*)pCodeBuffer, ushCodeLength);

    if(iRet < 0)
    {
        LotusStatistic::instance()->AddFailedPushNumber(1);
        return iRet;
    }

    return 0;
}

int CCodeQueueAssemble::PushToSpecific(int iCodeQueueID, const unsigned short ushCodeLength, const char* pCodeBuffer,
                                       const bool bIsRecordEnabled)
{
    int iRet = 0;

    if(iCodeQueueID < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CCodeQueueAssemble::PushToSpecific, CodeQueueID Error[%d]\n", iCodeQueueID);
        return 0;
    }

    iRet = GetSpecificCodeQueue(iCodeQueueID)->PushOneCode((const unsigned char*)pCodeBuffer, ushCodeLength);

    if(iRet < 0)
    {
        LotusStatistic::instance()->AddFailedPushNumber(1);
        return iRet;
    }

    if(bIsRecordEnabled)
    {
        PushToRecord(ushCodeLength, pCodeBuffer);
    }

    return 0;
}


int CCodeQueueAssemble::PopOneDefault(unsigned short& rushOutLength,char* &rpOutCodeBegin,
                  char* &rpOutCodeEnd, char* &rpHeadCode, char* &rpTailCode)
{
    int iRet = 0;

    rpHeadCode = (char*)m_pstDefaultOutputCodeQueue->GetHeadCodeQueue();
    rpTailCode = (char*)m_pstDefaultOutputCodeQueue->GetTailCodeQueue();

    iRet = m_pstDefaultOutputCodeQueue->GetOneCode((unsigned char* &)rpOutCodeBegin, (int&)rushOutLength);
    if(iRet < 0)
    {
        return iRet;
    }

    if(rushOutLength == 0)
    {
        return iRet;
    }

    iRet = m_pstDefaultOutputCodeQueue->PopOneCode();
    if(iRet < 0)
    {
        return iRet;
    }

    rpOutCodeEnd = rpOutCodeBegin + rushOutLength;

    return 0;
}
