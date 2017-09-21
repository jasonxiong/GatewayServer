#include "base.hpp"
#include "IOHandler.hpp"
#include "ShmObjectCreator.hpp"
#include "SignalWrapper.hpp"
#include "LotusLogAdapter.hpp"
#include "LotusStatistic.hpp"
#include "MsgStatistic.hpp"
#include "ExternalOutputThread.hpp"

const unsigned int SVR_VERSION = 20170628;


#define LOTUS_PID_FILE "./GatewayServer.pid"

//检查文件锁，防止重复运行
void CheckLock(const char* pszLockFile)
{
    int iLockFD = open(pszLockFile, O_RDWR | O_CREAT, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    if(iLockFD < 0)
    {
        printf("Open LockFile %s Failed, Server CheckLock Failed!\n", pszLockFile);
        exit(1);
    }

    if(flock(iLockFD, LOCK_EX | LOCK_NB) < 0)
    {
        printf("Server is already Running!\n");
        exit(1);
    }
}


//初始化为守护进程的函数
void DaemonLaunch(void)
{
    pid_t pid;
    if((pid = fork()) != 0)
    {
        exit(0);
    }

    setsid();

    CSignalWrapper::IgnoreSignalSet();

    if((pid = fork()) != 0)
    {
        exit(0);
    }

    umask(0);
}


//如进程正常退出，pid文件将被删除；否则为异常终止
//进程pid可被脚本用来关闭进程、重新读取配置文件
void WritePidFile(int id, const char* pszFilename)
{
    FILE* fp;

    fp = fopen(pszFilename, "w");
    if(NULL == fp)
    {
        printf("Failed to open pid file: %s\n", pszFilename);
    }

    fprintf(fp, "%d", id);

    fclose(fp);
}

// 读取PID文件
int ReadPidFile(const char* pszFilename)
{
    FILE* fp;

    fp = fopen(pszFilename, "r");
    if(NULL == fp)
    {
        printf("Failed to open pid file: %s\n", pszFilename);
        return -1;
    }

    char szPid[100];
    fread(szPid, sizeof(szPid), 1, fp);
    int iPid = atoi(szPid);

    fclose(fp);

    printf("ReadPid: %d\n", iPid);

    return iPid;
}

int main(int argc, char* *argv)
{
    char szVeriosnInfo[256] = {0};
    bool bDaemonLaunch = true;
    bool bNagelOff = true; // 默认关闭nagel算法

    if(argc > 1)
    {
        if(!strcasecmp(argv[1], "-v"))
        {
#ifdef _DEBUG_
            sprintf(&szVeriosnInfo[0], "GatewayServer %s Version:%s in %s %s\n",
                "DEBUG", GetCurVersion(SVR_VERSION), __DATE__, __TIME__);
#else
            sprintf(&szVeriosnInfo[0], "GatewayServer %s Version:%s in %s %s\n",
                "RUN", GetCurVersion(SVR_VERSION), __DATE__, __TIME__);
#endif

            printf("%s", &szVeriosnInfo[0]);
            exit(0);
        }
        else if(!strcasecmp(argv[1], "-d"))
        {
            bDaemonLaunch = false;
        }
        else if (!strcasecmp(argv[1], "-n")) // 打开收发套接字为nagel算法，默认是关闭的
        {
            bNagelOff = false; // 打开nagel算法
        }

        // 兼容TApp的stop/reload参数
        for (int i = 0; i < argc; i++)
        {
            if (!strcasecmp(argv[i], "stop"))
            {
                int iPid = ReadPidFile(LOTUS_PID_FILE);
                if (iPid > 0)
                {
                    kill(iPid, SIGUSR2);
                }

                exit(0);
            }
            else if (!strcasecmp(argv[i], "reload"))
            {
                int iPid = ReadPidFile(LOTUS_PID_FILE);
                if (iPid > 0)
                {
                    kill(iPid, SIGUSR1);
                }

                exit(0);
            }
        }
    }

    CheckLock(".App.lock");

    if(bDaemonLaunch)
    {
        DaemonLaunch();
    }

    WritePidFile(getpid(),LOTUS_PID_FILE);

    CSharedMemory m_stSharedMemory;
    char szKeyFileName[256] = {0};
    snprintf(szKeyFileName, sizeof(szKeyFileName) - 1, "./LotusServer.shm");
    char szCmd[512] = {0};
    sprintf(szCmd, "touch %s", szKeyFileName);
    system(szCmd);
    CIOHandler* m_pstIOHandler = CShmObjectCreator<CIOHandler>::CreateObject(&m_stSharedMemory, "./LotusServer.shm", 'a');
    if(!m_pstIOHandler)
    {
        printf("CreateObject LotusServer Failed!\n");
        return -1;
    }

    CSignalWrapper::SetHandler_USR1(CIOHandler::SetAppCmd, APPCMD_RELOAD_CONFIG);
    CSignalWrapper::SetHandler_USR2(CIOHandler::SetAppCmd, APPCMD_STOP_SERVICE);

    int iRet = 0;
    iRet = m_pstIOHandler->Initialize(bNagelOff);
    if(iRet < 0)
    {
        printf("m_pstIOHandler Initialize Failed(%d)!\n", iRet);
        return -2;
    }

    iRet = m_pstIOHandler->CreateEpoll();
    if(iRet < 0)
    {
        printf("m_pstIOHandler CreateEpoll Failed(%d)!\n", iRet);
        return -3;
    }

    iRet = m_pstIOHandler->CreateAllListener();
    if(iRet < 0)
    {
        printf("m_pstIOHandler CreateAllListener Failed(%d)!\n", iRet);
        return -4;
    }

#ifdef ENABLE_EXTERNAL_OUTPUT_THREAD
    CExternalOutputThread m_stExternalOutputThread;
    iRet = m_stExternalOutputThread.Initialize(m_pstIOHandler);
    if (iRet < 0)
    {
        printf("External Output Thread Failed(%d)!\n", iRet);
        return -5;
    }
#endif

    LotusStatistic::instance()->ClearAllStat();
    MsgStatisticSingleton::instance()->Initialize();
    MsgStatisticSingleton::instance()->Reset();

    unsigned int uiNowTime = time(NULL);
    unsigned int uiStatTime = time(NULL);

    while(true)
    {
        m_pstIOHandler->CheckIO();
        m_pstIOHandler->CheckCodeQueue();
        m_pstIOHandler->CheckAppCmd();
        m_pstIOHandler->CheckTimeOut();

        //统计
        uiNowTime = time(NULL);
        if(uiNowTime - uiStatTime >= 60)
        {
            MsgStatisticSingleton::instance()->Print();
            MsgStatisticSingleton::instance()->Reset();
            TRACESTAT("================Begin Statistic================\n");
            LotusStatistic::instance()->RecordAllStat(uiNowTime - uiStatTime);
            LotusStatistic::instance()->ClearAllStat();

            uiStatTime = uiNowTime;
            TRACESTAT("================End Statistic================\n");
        }
    }

    unlink(LOTUS_PID_FILE);

    return 0;
}

