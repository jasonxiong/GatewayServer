#include "ExternalOutputThread.hpp"

void* ThreadProc( void *pvArgs )
{
    if( !pvArgs )
    {
        return NULL;
    }

    CExternalOutputThread *pThread = (CExternalOutputThread *)pvArgs;

    //run the thread logic
    pThread->Run();

    return NULL;
}

int CExternalOutputThread::Initialize(CIOHandler* pIOHandler)
{
    m_pstIOHandler = pIOHandler;

    return CreateThread();
}

int CExternalOutputThread::Run()
{
#ifdef ENABLE_TNM2_REPORT
    int iLastStaticTime = time(NULL);
#endif
    while (1)
    {
        m_pstIOHandler->CheckDefaultCodeQueue();

#ifdef ENABLE_TNM2_REPORT

        int iNow = time(NULL);
        if (iNow - iLastStaticTime >= 60)
        {
            // 下行通道消息量
            CCodeQueue* pOutputCodeQueue = m_pstIOHandler->GetCodeQueueAssemble().m_pstDefaultOutputCodeQueue;
            if (pOutputCodeQueue)
            {
                TNM2_REPORT_VALUE(14663, pOutputCodeQueue->GetUsedLength());
            }

            iLastStaticTime = iNow;
        }

#endif

        usleep(10);
    }

    return 0;
}


int CExternalOutputThread::CreateThread()
{
    int iRt = 0;
    iRt = pthread_attr_init(&m_stAttr);
    if (iRt != 0)
    {
        return -1;
    }

    // to complete with system thread
    iRt = pthread_attr_setscope(&m_stAttr, PTHREAD_SCOPE_SYSTEM);
    if (iRt != 0)
    {
        return -3;
    }

    iRt = pthread_attr_setdetachstate(&m_stAttr, PTHREAD_CREATE_JOINABLE);
    if (iRt != 0)
    {
        return -5;
    }

    iRt = pthread_create(&m_hTrd, &m_stAttr, ThreadProc, (void *)this);
    if (iRt != 0)
    {
        return -7;
    }

    return 0;
}

