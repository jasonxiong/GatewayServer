
#ifndef __CODEQUEUE_ASSEMBLE_HPP__
#define __CODEQUEUE_ASSEMBLE_HPP__



#include "SharedMemory.hpp"
#include "LotusDefine.hpp"
#include "ConfigDefine.hpp"

#include "CodeQueue.hpp"


//class CCodeQueue;

class CCodeQueueAssemble
{
public:

    int CreateCodeQueue(const TCodeQueueConfig& rstCodeQueueConfig);

    int Initialize();

    //发送到默认业务CodeQueue
    int PushToDefault(const unsigned short ushCodeLength, const char* pCodeBuffer,
        const bool bIsRecordEnabled);

    int PushToSpecific(int iCodeQueueID, const unsigned short ushCodeLength, const char* pCodeBuffer,
        const bool bIsRecordEnabled);

    int PopOneDefault(unsigned short& rushOutLength,char* &rpOutCodeBegin,
        char* &rpOutCodeEnd, char* &rpHeadCode, char* &rpTailCode);

    //All
    int m_iCodeQueueNumber;
    CCodeQueue* m_apstCodeQueue[MAX_CODEQUEUE_NUMBER];
    CSharedMemory m_astSharedMemory[MAX_CODEQUEUE_NUMBER];

    //Input
    CCodeQueue* m_pstDefaultInputCodeQueue;
    CCodeQueue* m_pstRecordInputCodeQueue;

    //unsigned int m_uiDefaultNumber;
    //CCodeQueue* m_apstDefaultCodeQueue[MAX_CODEQUEUE_NUMBER];

    //Output
    int m_iOutputCodeQueueNumber;
    CCodeQueue* m_apstOutputCodeQueue[MAX_CODEQUEUE_NUMBER];
    bool m_abOutputCodeQueueEmpty[MAX_CODEQUEUE_NUMBER];
    int m_aiOutputCodeQueueID[MAX_CODEQUEUE_NUMBER];

    CCodeQueue* m_pstDefaultOutputCodeQueue;
    CCodeQueue* m_pstRecordOutputCodeQueue;

private:
    int PushToRecord(const unsigned short ushCodeLength, const char* pCodeBuffer);

    CCodeQueue* GetSpecificCodeQueue(int iCodeQueueID){return m_apstCodeQueue[iCodeQueueID];};

};

#endif
