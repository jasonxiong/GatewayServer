#include <algorithm>
#include <string.h>

#include "base.hpp"
#include "SocketOperator.hpp"
#include "ShmObjectCreator.hpp"
#include "IOHandler.hpp"
#include "LotusLogAdapter.hpp"
#include "LotusStatistic.hpp"
#include "BillWriter.hpp"
#include "TimeTools.hpp"

int CIOHandler::m_iAppCmd = 0;
CIOHandler* CIOHandler::m_pstThisHandler = 0;

const unsigned int SEND_MULTI_SESSION = 0xFFFFFFFF;
const unsigned int SEND_ALL_SESSION = 0xFFFFFFFF;
const unsigned int SEND_ALL_SESSION_BUTONE = 0xFFFFFFFE;

int CIOHandler::Initialize(bool bNagelOff)
{
    m_bNagelOff = bNagelOff;
    TRACESVR(LOG_LEVEL_DETAIL, "Nagel off: %s\n", m_bNagelOff ? "true" : "false");

    int iRet = 0;

    iRet = m_stConfigAssemble.LoadConfig();
    if(iRet < 0)
    {
        printf("CIOHandler::Initialize Initialize Failed(%d)!\n", iRet);
        TRACESVR(LOG_LEVEL_DETAIL, "CIOHandler::Initialize Initialize Failed(%d)!\n", iRet);

        return -2;
    }

    //初始化防过载
    m_stSafeguradStrategy.Initialize(&m_stSafegurad, FILE_LOTUSCONFIG_SERVER);
    m_stSafegurad.SetStrategy(&m_stSafeguradStrategy);
    m_stSafegurad.Reset();

    //初始化CodeQueue集合
    m_stCodeQueueAssemble.Initialize();

    //创建CodeQueue
    int i = 0;
    for(i = 0; i < m_stConfigAssemble.GetLotusConfigMng().GetCodeQueueNumber(); ++i)
    {
        const TCodeQueueConfig& rstConfig = m_stConfigAssemble.GetLotusConfigMng().GetCodeQueueByIndex(i);
        m_stCodeQueueAssemble.CreateCodeQueue(rstConfig);
    }

    //创建FDPool
    char szKeyFileName[256] = {0};
    snprintf(szKeyFileName, sizeof(szKeyFileName) - 1, "./FDPool.shm");
    char szCmd[512] = {0};
    sprintf(szCmd, "touch %s", szKeyFileName);
    system(szCmd);
    m_pstFDPool = CShmObjectCreator<CFDPool>::CreateObject(&m_stSharedMemoryFDPool, "./FDPool.shm", 'p');
    if(!m_pstFDPool)
    {
        return -2;
    }
    m_pstFDPool->Initialize();

    //初始化报文派发器
    m_stCodeDispatcher.Initialize();
    m_stCodeDispatcher.m_pIOHandler = this;
    m_stCodeDispatcher.m_pstConfigAssemble = &m_stConfigAssemble;
    m_stCodeDispatcher.m_pstCodeQueueAssemble = &m_stCodeQueueAssemble;
    m_stCodeDispatcher.m_pstFDPool = m_pstFDPool;

    //初始化AppCmd
    m_iAppCmd = 0;
    m_tLastCheckTimeout = 0;
    m_tLastCheckHello = 0;
    m_tLastSafeguradTimeout = 0;

    m_pstThisHandler = this;

    return 0;
}

int CIOHandler::CreateEpoll()
{
    int iRet = 0;

    //创建Epoll
    iRet = m_stEpollWrapper.EpollCreate(FD_SIZE);
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::CreateEpoll EpollCreate Ret:%d\n", iRet);
        return -1;
    }

    //设置Epoll事件检查时间为4毫秒
    m_stEpollWrapper.SetEpollWaitingTime(4);

    //注册Epoll输入输出处理者
    m_stEpollWrapper.SetHandler_Error(CIOHandler::OnError);
    m_stEpollWrapper.SetHandler_Read(CIOHandler::OnRead);
    m_stEpollWrapper.SetHandler_Write(CIOHandler::OnWrite);

    return 0;
}

int CIOHandler::CreateAllListener()
{
    int i = 0;
    m_uiListenerNumber = 0;

    //创建外部监听器
    const TAddressGroup& rstExternalAddressGroup = m_stConfigAssemble.GetLotusConfigMng().GetExternalListenerAddress();
    for(i = 0; i < (int)rstExternalAddressGroup.m_uiNumber; ++i)
    {
        int iRet = CreateOneListener(rstExternalAddressGroup.m_astAddress[i].m_uiIP, rstExternalAddressGroup.m_astAddress[i].m_ushPort,
            m_stConfigAssemble.GetLotusConfigMng().GetInternalBufferSize(), true);
        if (iRet < 0)
        {
            return -1;
        }
    }

    //创建内部监听器
    const TAddressGroup& rstInternalAddressGroup = m_stConfigAssemble.GetLotusConfigMng().GetInternalListenerAddress();
    for(i = 0; i < (int)rstInternalAddressGroup.m_uiNumber; ++i)
    {
        int iRet = CreateOneListener(rstInternalAddressGroup.m_astAddress[i].m_uiIP, rstInternalAddressGroup.m_astAddress[i].m_ushPort,
            m_stConfigAssemble.GetLotusConfigMng().GetInternalBufferSize(), false);
        if (iRet < 0)
        {
            return -2;
        }
    }

    return 0;
}

int CIOHandler::CreateOneListener(unsigned int uiIP, unsigned short ushPort, unsigned int uiSendBufferSize,
                                  bool bExternalClient)
{
    int iRet = -1;

    CLotusListener& rstLotusListener = m_astLotusListener[m_uiListenerNumber];
    rstLotusListener.Init(uiIP, ushPort, uiSendBufferSize, bExternalClient);

    //创建套接字
    iRet = rstLotusListener.CreateTCPSocket();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::CreateOneListener CreateTCPSocket Ret:%d\n", iRet);
        return -1;
    }

    //设置Epoll适用的套接字参数
    iRet = rstLotusListener.SetForEpoll();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::CreateOneListener SetForEpoll Ret:%d\n", iRet);
        return -1;
    }

    //设置发送缓冲区尺寸
    // 这里不设置缓冲区大小, 采用系统默认值
//  iRet = rstLotusListener.SetSendBufferSize(uiSendBufferSize);
//  if(iRet < 0)
//  {
//         TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::CreateOneListener SetSendBufferSize Ret:%d\n", iRet);
//      return -1;
//  }

    //绑定地址
    iRet = rstLotusListener.Bind(uiIP, ushPort);
    if(iRet < 0)
    {
        sockaddr_in stAddr;
        stAddr.sin_addr.s_addr = uiIP;

        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::CreateOneListener Bind on %s:%d Ret:%d\n", inet_ntoa(stAddr.sin_addr), ushPort, iRet);
        return -1;
    }

    //开始监听
    iRet = rstLotusListener.Listen();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::CreateOneListener Listen Ret:%d\n", iRet);
        return -1;
    }

    int iFD = rstLotusListener.GetFD();

    //加入Epoll管理
    AddFD(iFD);

    if(bExternalClient)
    {
        m_pstFDPool->SetListeningForExternalClient(iFD);
    }
    else
    {
        m_pstFDPool->SetListeningForInternalServer(iFD);
    }

    m_uiListenerNumber++;

    TRACESVR(LOG_LEVEL_NOTICE, "In CIOHandler::CreateOneListener Add Listener FD:%d IP:%u Port:%d OK\n",
        iFD, uiIP, ushPort);

    return 0;
}

CLotusListener* CIOHandler::GetListenerByFD(int iListeningSocket)
{
    if (iListeningSocket < 0)
    {
        return NULL;
    }

    CLotusListener *pstLotusListener = NULL;

    for (int i = 0; i < (int)m_uiListenerNumber; i++)
    {
        pstLotusListener = &m_astLotusListener[m_uiListenerNumber];
        if (iListeningSocket == pstLotusListener->GetFD())
        {
            return pstLotusListener;
        }
    }

    return NULL;
}

int CIOHandler::ReCreateOneListener(CLotusListener *pstLotusListener)
{
    if (NULL == pstLotusListener)
    {
        return -1;
    }

    int iRet = -1;

    iRet = pstLotusListener->CreateTCPSocket();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::ReCreateOneListener CreateTCPSocket Ret:%d\n", iRet);
        return -1;
    }

    // 设置Epoll适用的套接字参数
    iRet = pstLotusListener->SetForEpoll();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::ReCreateOneListener SetForEpoll Ret:%d\n", iRet);
        return -1;
    }

    // 绑定地址
    iRet = pstLotusListener->Bind(pstLotusListener->m_uiIP, pstLotusListener->m_ushPort);
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::ReCreateOneListener Bind Ret:%d\n", iRet);
        return -1;
    }

    // 开始监听
    iRet = pstLotusListener->Listen();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::ReCreateOneListener Listen Ret:%d\n", iRet);
        return -1;
    }

    int iFD = pstLotusListener->GetFD();

    // 加入Epoll管理
    AddFD(iFD);

    if(pstLotusListener->m_bExternalClient)
    {
        m_pstFDPool->SetListeningForExternalClient(iFD);
    }
    else
    {
        m_pstFDPool->SetListeningForInternalServer(iFD);
    }

    TRACESVR(LOG_LEVEL_NOTICE, "In CIOHandler::ReCreateOneListener Add Listener FD:%d IP:%u Port:%d OK\n",
        iFD, pstLotusListener->m_uiIP, pstLotusListener->m_ushPort);

    return 0;
}

int CIOHandler::CheckIO()
{
    m_stEpollWrapper.EpollWait();

    // 检查Listen状态
    for (int i = 0; i < (int)m_uiListenerNumber; i++)
    {
        CLotusListener *pstLotusListener = &m_astLotusListener[i];
        if (LotusListener_UnListen == pstLotusListener->m_iState)
        {
            ReCreateOneListener(pstLotusListener);
        }
    }

    return 0;
}

int CIOHandler::CheckOffline()
{
    time_t tNow = time(0);

    //检查外部连接是否断线
    int iFD;
    int iExternalConnectionNumber = 0;
    int iResetNumber = 0;

    for(iFD = 0; iFD < (int)FD_SIZE; ++iFD)
    {
        //如果是外部连接
        if(m_pstFDPool->IsConnectedByExternalClient(iFD))
        {
            TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
            if(pstExternalClientSocket == NULL)
            {
                continue;
            }

            ++iExternalConnectionNumber;

            //初始化频率限制
            pstExternalClientSocket->m_uiRecvPacketNumber = 0;

            //如果未收过后台的响应包，则超时断线的时间更短，防止恶意攻击
            if(pstExternalClientSocket->m_iSendFlag == ESCF_NO_PACKET_TO_CLIENT)
            {
                if(tNow - (int)pstExternalClientSocket->m_uiCreateTime >=
                    m_stConfigAssemble.GetLotusConfigMng().GetIdleConnectionTimeout())
                {
                    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::CheckTimeOut, iFD(%d) is closed by not having Server rsp.\n", iFD);
                    Reset(iFD, TIMEOUT_SVR_NOT_RSP);
                    ++iResetNumber;
                }
            }
            else
            {
                //有响应包，则判断客户端上一次发消息的时间，判断是否掉线了
                if(tNow - (int)pstExternalClientSocket->m_uiRecvClientTimeStamp >=
                    m_stConfigAssemble.GetLotusConfigMng().GetIdleClientTimeout())
                {
                    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::CheckTimeOut, iFD(%d) is closed by not recving message too long.\n", iFD);
                    Reset(iFD, TIMEOUT_IDLE_CLIENT);
                    ++iResetNumber;
                }
                //长时间没收到响应包
                else if(tNow - (int)pstExternalClientSocket->m_uiRecvSvrTimeStamp >=
                    m_stConfigAssemble.GetLotusConfigMng().GetIdleServerTimeout())
                {
                    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::CheckTimeOut, iFD(%d) is closed by Server not rsp message too long.\n", iFD);
                    Reset(iFD, TIMEOUT_IDLE_SVR);
                    ++iResetNumber;
                }
            }
        }
    }

    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::CheckOffline, Check %d External Connect, Reset %d.\n",
        iExternalConnectionNumber, iResetNumber);

    return 0;
}

int CIOHandler::CheckTimeOut()
{
    time_t tNow = time(0);

    if(tNow - m_tLastCheckTimeout >= 60)
    {
        m_tLastCheckTimeout = tNow;

        //在这里做断线检查
        CheckOffline();
    }

    if(tNow - m_tLastSafeguradTimeout >= 60)
    {
        m_tLastSafeguradTimeout = tNow;

        PrintStat();

        m_stSafegurad.Timeout();
    }

    return 0;
}

int CIOHandler::OnError(int iFD)
{
    m_pstThisHandler->Reset(iFD, ERROR_NET_IO);

    CFDPool* pFDPool = m_pstThisHandler->m_pstFDPool;
    if (NULL != pFDPool)
    {
        if(pFDPool->IsListeningForExternalClient(iFD) || pFDPool->IsListeningForInternalServer(iFD))
        {
            // 说明listen出错  那么重新建立监听
            // 如果这次错误重新建立不了Listen
            // 只能放循环里面做了？
            CLotusListener *pstLotusListener = m_pstThisHandler->GetListenerByFD(iFD);
            if (NULL != pstLotusListener)
            {
                pstLotusListener->m_iState = LotusListener_UnListen;

                // 在 CheckIO中统一发起吧
                // m_pstThisHandler->ReCreateOneListener(pstLotusListener);
            }
        }
    }

    return 0;
}

int CIOHandler::OnRead(int iFD)
{
    int iRet;

    CFDPool* pFDPool = m_pstThisHandler->m_pstFDPool;

    //来自监听端口
    if(pFDPool->IsListeningForExternalClient(iFD) || pFDPool->IsListeningForInternalServer(iFD))
    {
        iRet = m_pstThisHandler->Accept(iFD);
    }
    //来自外部
    else if(pFDPool->IsConnectedByExternalClient(iFD))
    {
        iRet = m_pstThisHandler->OnReadFromExternal(iFD);
    }
    //来自内部
    else if(pFDPool->IsConnectedByInternalServer(iFD))
    {
        iRet = m_pstThisHandler->OnReadFromInternal(iFD);
    }

    return 0;
}

int CIOHandler::OnWrite(int iFD)
{
    TRACESVR(LOG_LEVEL_NOTICE, "In CIOHandler::OnWrite, FD:%d OK\n", iFD);

    return 0;
}

int CIOHandler::AddFD(int iFD)
{
    m_stEpollWrapper.EpollAdd(iFD);
    m_stEpollWrapper.SetNonBlock(iFD);

    if (m_bNagelOff)
    {
        m_stEpollWrapper.SetNagleOff(iFD);
    }

    m_pstFDPool->SetFDActive(iFD);
    return 0;
}

int CIOHandler::PrintStat()
{
    TRACESTAT("CurConnnectNum:%d, NewConnectNum:%d, CloseConnectNum:%d, C2SPacketNum:%d, S2CPacketNum:%d\n",
        m_stSafegurad.m_iCurExternalConnectNum, m_stSafegurad.m_iCurNewConnectNum,
        m_stSafegurad.m_iCurNewCloseConnectNum,
        m_stSafegurad.m_iCurUpPackCounter, m_stSafegurad.m_iCurDownPacketCounter);

    return 0;
}

int CIOHandler::Accept(int iListeningFD)
{
    unsigned int uiRemoteIP = 0;
    unsigned short ushRemotePort = 0;
    int iFD = -1;

    iFD = CSocketOperator::Accept(iListeningFD, uiRemoteIP, ushRemotePort);
    if(iFD < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::Accept Accept Ret:%d\n", iFD);
        return -1;
    }

    if(!OnAccept(uiRemoteIP))
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::Accept OnAccept Failed!\n");
        CSocketOperator::Close(iFD);
        return -2;
    }

    //如果是内部监听端口
    if(m_pstFDPool->IsListeningForInternalServer(iListeningFD))
    {
        //获取监听端口,区分同IP的不同连接
        struct sockaddr_in sockadd;
        socklen_t len = sizeof(struct sockaddr_in);
        getsockname(iListeningFD, (struct sockaddr *)&sockadd, &len);
        unsigned short ushListenedPort = ntohs(sockadd.sin_port);

        //看看是否在授信名单中
        TInternalServerSocket* pstInternalServerSocket = m_pstFDPool->GetInternalSocketByInternalServerIP(uiRemoteIP, ushListenedPort);
        if(!pstInternalServerSocket)
        {
            TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::Accept Connect InternalServer[%u %u] Not In Config!\n", uiRemoteIP, ushListenedPort);
            CSocketOperator::Close(iFD);
            return -3;
        }

        //必须关闭其他来源IP相同的连接
        int iPreFD = pstInternalServerSocket->m_iSocketFD;
        if(iPreFD >= 0 && iPreFD < (int)FD_SIZE)
        {
            if(m_pstFDPool->IsConnectedByInternalServer(iPreFD))
            {
                Reset(iPreFD, ERROR_INTERNAL_CONN);
            }
        }

        pstInternalServerSocket->m_iSocketFD = iFD;
        pstInternalServerSocket->m_uiSrcIP = uiRemoteIP;
        pstInternalServerSocket->m_uiRecvEndOffset = 0;
        pstInternalServerSocket->m_uiSendEndOffset = 0;

        m_pstFDPool->SetConnectedByInternalServer(iFD);

        //设置发送缓冲尺寸
        int iOptLength = sizeof(socklen_t);
        int iOptValue = m_stConfigAssemble.GetLotusConfigMng().GetInternalBufferSize();

        if(setsockopt(iFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptValue, iOptLength))
        {
            TRACESVR(LOG_LEVEL_DETAIL, "Set Send Buffer Size to %d Failed!\n", iOptValue);
            return -4;
        }
        if(!getsockopt(iFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptValue, (socklen_t *)&iOptLength))
        {
            TRACESVR(LOG_LEVEL_NOTICE, "Set Send Buffer of Socket:%d.\n", iOptValue);
        }

        if(setsockopt(iFD, SOL_SOCKET, SO_RCVBUF, (const void *)&iOptValue, iOptLength))
        {
            TRACESVR(LOG_LEVEL_DETAIL, "Set Send Buffer Size to %d Failed!\n", iOptValue);
            return -5;
        }
        if(!getsockopt(iFD, SOL_SOCKET, SO_RCVBUF, (void *)&iOptValue, (socklen_t *)&iOptLength))
        {
            TRACESVR(LOG_LEVEL_NOTICE, "Set Send Buffer of Socket:%d.\n", iOptValue);
        }
    }
    else
    {
        TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
        if(!pstExternalClientSocket)
        {
            CSocketOperator::Close(iFD);
            return -6;
        }

        //如果受防过载保护
        if(!m_stSafegurad.IsAcceptConnect())
        {
            m_stSafegurad.IncreaseRefuseAcceptNum();
            CSocketOperator::Close(iFD);
            return -7;
        }

        m_stSafegurad.IncreaseNewConnectNum();

        pstExternalClientSocket->m_iSocketFD = iFD;
        pstExternalClientSocket->m_uiSrcIP = uiRemoteIP;
        pstExternalClientSocket->m_ushSrcPort = ushRemotePort;
        pstExternalClientSocket->m_uiCreateTime = time(NULL);
        pstExternalClientSocket->m_uiRecvClientTimeStamp = 0;
        pstExternalClientSocket->m_uiRecvSvrTimeStamp = 0;
        pstExternalClientSocket->m_iSendFlag = 0;
        pstExternalClientSocket->m_uiRecvEndOffset = MAX_NETHEAD_SIZE;
        pstExternalClientSocket->m_uiUin = 0;
        pstExternalClientSocket->m_uiRecvPacketNumber = 0;

        pstExternalClientSocket->m_uiSocketFD_NBO = htonl((unsigned int)pstExternalClientSocket->m_iSocketFD);
        pstExternalClientSocket->m_uiSrcIP_NBO = htonl(pstExternalClientSocket->m_uiSrcIP);
        pstExternalClientSocket->m_ushSrcPort_NBO = htons(pstExternalClientSocket->m_ushSrcPort);
        pstExternalClientSocket->m_uiCreateTime_NBO = htonl(pstExternalClientSocket->m_uiCreateTime);

        m_pstFDPool->SetConnectedByExternalClient(iFD);

        if(m_stConfigAssemble.GetLotusConfigMng().GetBillFlag() & EBF_ACCEPT_SOCKET)
        {
            CBillWriter::WriteSocketBill(pstExternalClientSocket, EBEI_ACCEPT_SOCKET);
        }

        m_stSafegurad.IncreaseExternalConnectNum();

        //设置发送缓冲尺寸
        int iOptLength = sizeof(socklen_t);
        int iOptValue = m_stConfigAssemble.GetLotusConfigMng().GetExternalSNDBufferSize();

        if(setsockopt(iFD, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptValue, iOptLength))
        {
            TRACESVR(LOG_LEVEL_DETAIL, "Set Send Buffer Size to %d Failed!\n", iOptValue);
            return -4;
        }

        if(!getsockopt(iFD, SOL_SOCKET, SO_SNDBUF, (void *)&iOptValue, (socklen_t *)&iOptLength))
        {
            //TRACESVR(LOG_LEVEL_NOTICE, "Set Send Buffer of Socket:%d.\n", iOptValue);
        }

        iOptValue = m_stConfigAssemble.GetLotusConfigMng().GetExternalRCVBufferSize();

        if(setsockopt(iFD, SOL_SOCKET, SO_RCVBUF, (const void *)&iOptValue, iOptLength))
        {
            TRACESVR(LOG_LEVEL_DETAIL, "Set Send Buffer Size to %d Failed!\n", iOptValue);
            return -5;
        }

        if(!getsockopt(iFD, SOL_SOCKET, SO_RCVBUF, (void *)&iOptValue, (socklen_t *)&iOptLength))
        {
            //TRACESVR(LOG_LEVEL_NOTICE, "Set Send Buffer of Socket:%d.\n", iOptValue);
        }
    }

    AddFD(iFD);

    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::Accept New Connection FD:%d\n", iFD);

    return 0;
}

int CIOHandler::OnReadFromInternal(int iFD)
{
    int iRet = 0;

    TInternalServerSocket* pstSocket = m_pstFDPool->GetInternalSocketByFD(iFD);
    if(!pstSocket)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromInternal, pstSocket is NULL, FD:%d\n", iFD);
        return -1;
    }

    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromInternal, Some Date From ServerType:%d ServerID:%d FD:%d Before Recv\n",
        pstSocket->m_ushServerType, pstSocket->m_ushServerID, iFD);

    iRet = CSocketOperator::Recv(iFD, sizeof(pstSocket->m_szRecvBuffer) - pstSocket->m_uiRecvEndOffset,
        &pstSocket->m_szRecvBuffer[0] + pstSocket->m_uiRecvEndOffset);

    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromInternal, On Recv FD:%d Recv Byte:%d\n", iFD, iRet);

    if(iRet < 0)
    {
        Reset(iFD, ERROR_INTERNAL_IO_READ);
        return -2;
    }

    pstSocket->m_uiRecvEndOffset += iRet;

    iRet = DispatchFromInternalServer(iFD);
    if (iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromInternal, DispatchFromInternalServer:%d\n", iRet);
        Reset(iFD, ERROR_DISPATCH_FROM_INTERNAL_SVR);
        return -3;
    }

    return 0;
}

int CIOHandler::OnReadFromExternal(int iFD)
{
    int iRet = 0;

    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
    if(!pstExternalClientSocket)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromExternal, pstExternalClientSocket=NULL\n");
        return -1;
    }

    //防刷
    if((int)pstExternalClientSocket->m_uiRecvPacketNumber >= m_stConfigAssemble.GetLotusConfigMng().GetClientPacketMaxFrequencyNum())
    {
        //接收失败或者关闭
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromExternal, RecvReqLimit Overload, Close Socket, Uin:%u\n",
            pstExternalClientSocket->m_uiUin);
        Reset(iFD, ERROR_EXTERAL_RECV_REQ_OVERLOAD);
        return -2;
    }

    //防过载限制
    if(!m_stSafegurad.IsAcceptReceive())
    {
        m_stSafegurad.IncreaseRefuseReciveNum();
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromExternal, AcceptReceive Overload, Close Socket\n");
        Reset(iFD, ERROR_EXTERAL_ACCEPT_RECV_OVERLOAD);
        return -3;
    }

    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, Before Recv FD:%d RecvEndOffset:%u\n",
        iFD, pstExternalClientSocket->m_uiRecvEndOffset);

    iRet = CSocketOperator::Recv(iFD, sizeof(pstExternalClientSocket->m_szRecvBuffer) - pstExternalClientSocket->m_uiRecvEndOffset,
        &pstExternalClientSocket->m_szRecvBuffer[0] + pstExternalClientSocket->m_uiRecvEndOffset);

    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, On Recv FD:%d Recv Byte:%d\n", iFD, iRet);

    if(iRet < 0)
    {
        //接收失败或者关闭
        if(iRet == -2)
        {
            TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, Client Close FD:%d, So Reset\n", iFD);
        }

        TRACESVR(LOG_LEVEL_ALERT, "Recv Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d\n", __LINE__,
            pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
            pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet);
        Reset(iFD, ERROR_EXTERAL_RECV);
        return -4;
    }

    //统计
    m_stSafegurad.IncreaseUpPacketNum();
    LotusStatistic::instance()->AddInputPacket(iRet);

    //单个包长限制
    if((unsigned int)iRet > m_stConfigAssemble.GetLotusConfigMng().GetClientPacketMaxLength())
    {
        Reset(iFD, ERROR_EXTERAL_RECV_PACKET_TOO_MORE);
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromExternal, ClientPacketMaxLength:%d\n", iRet);
        return -5;
    }

    //更新收到包的时间戳
    pstExternalClientSocket->m_uiRecvClientTimeStamp = time(0);
    pstExternalClientSocket->m_uiRecvPacketNumber++;
    pstExternalClientSocket->m_uiRecvEndOffset += iRet;

    //不足一个short
    if(pstExternalClientSocket->m_uiRecvEndOffset < (MAX_NETHEAD_SIZE + sizeof(int)) )
    {
        return 0;
    }

    //下发Flash策略文件
    if(m_stConfigAssemble.GetLotusConfigMng().IsFlashCodeEnabled())
    {
        //判断是否是获取Flash策略文件的请求(发送的请求里最后包含了一个\0)
        if(pstExternalClientSocket->m_uiRecvEndOffset - MAX_NETHEAD_SIZE >= 
            (unsigned int)m_stConfigAssemble.GetLotusConfigMng().GetFlashReqBuf().m_shLength)
        {
            if(!strncmp(&pstExternalClientSocket->m_szRecvBuffer[MAX_NETHEAD_SIZE], 
                m_stConfigAssemble.GetLotusConfigMng().GetFlashReqBuf().m_szContent, 
                m_stConfigAssemble.GetLotusConfigMng().GetFlashReqBuf().m_shLength))
            {
                //相同,是策略文件请求

                TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, Now Send Flash Rsp\n");

                short shRspMsgLength = m_stConfigAssemble.GetLotusConfigMng().GetFlashRspBuf().m_shLength + 1;//需返回字符串结束符
                int iRet = CSocketOperator::Send(iFD, shRspMsgLength, m_stConfigAssemble.GetLotusConfigMng().GetFlashRspBuf().m_szContent);
                if(iRet < 0)
                {
					TRACESVR(LOG_LEVEL_ALERT, "Send Err:iRet=%d,Errno=%d,ErrStr=%s\n", iRet, errno, strerror(errno));
                    TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromExternal, Send Flash Rsp iRet:%d\n", iRet);
                }

				LotusStatistic::instance()->IncreaseOutputFlashNumber();
				m_stSafegurad.IncreaseDownPacketNum();
                TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, Flash Rsp Send OK\n");

                Reset(iFD, RSP_FLASH_STRATEGY_FILE);
                return 0;
            }
            else
            {
                //不同,是正常code,不做处理
            }
        }
        else
        {
            //不足长度的比较前N个字节
            if(!strncmp(&pstExternalClientSocket->m_szRecvBuffer[MAX_NETHEAD_SIZE],
                m_stConfigAssemble.GetLotusConfigMng().GetFlashReqBuf().m_szContent, 
                pstExternalClientSocket->m_uiRecvEndOffset - MAX_NETHEAD_SIZE))
            {
                TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, Match Some Flash Req, Continue Recv\n");
                //相同,也许是策略文件,返回继续收
                return 0;
            }
            else
            {
                //不同,是正常code,不做处理
            }
        }
    }

    //正常流程
    unsigned short ushTempCodeLength = ntohs(*(short*)&pstExternalClientSocket->m_szRecvBuffer[MAX_NETHEAD_SIZE]);
    TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::OnReadFromExternal, FD:%d CodeLength:%d RecvedLength:%d\n",
            iFD, ushTempCodeLength, pstExternalClientSocket->m_uiRecvEndOffset - MAX_NETHEAD_SIZE);

    if(ushTempCodeLength <= pstExternalClientSocket->m_uiRecvEndOffset - MAX_NETHEAD_SIZE)
    {
        iRet = DispatchFromExternalClient(iFD);
        if(iRet < 0)
        {
            TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::OnReadFromExternal, DispatchFromExternalClient Ret:%d\n", iRet);
            Reset(iFD, ERROR_DISPATCH_FROM_EXTERNAL_CLIENT);
            return -9;
        }
    }



    return 0;
}


void CIOHandler::CheckAppCmd()
{
    if(m_iAppCmd == APPCMD_STOP_SERVICE)
    {
        StopService();
    }
    if(m_iAppCmd == APPCMD_RELOAD_CONFIG)
    {
        ReloadConfig();
    }

    m_iAppCmd = 0;
}

void CIOHandler::SetAppCmd(const int iAppCmd)
{
    m_iAppCmd = iAppCmd;
}

void CIOHandler::StopService()
{
    exit(0);
}

void CIOHandler::ReloadConfig()
{
    int iRet = 0;

    iRet = m_stConfigAssemble.ReloadConfig();
    if(iRet < 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "CIOHandler::ReloadConfig Reload Failed\n");
        return;
    }

    m_stSafeguradStrategy.Initialize(&m_stSafegurad, FILE_LOTUSCONFIG_SERVER);

    // reread LogSetting.tcm
    LotusServerLogEngineSingleton::instance()->InitLogPath();
}

int CIOHandler::DispatchFromExternalClient(int iFD)
{
    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
    if(!pstExternalClientSocket)
    {
        return -1;
    }

    unsigned short ushCodeOffset = MAX_NETHEAD_SIZE;
    char* pszCodeBuffer = &pstExternalClientSocket->m_szRecvBuffer[MAX_NETHEAD_SIZE];
    unsigned short ushCodeLength = 0;
    int iRet = 0;

    while(true)
    {
        //如果收到的数据不足sizeofshort，那么不处理，移动已经接受的数据到MAX_NETHEAD_SIZE，以便继续接收数据
        if(pstExternalClientSocket->m_uiRecvEndOffset -  ushCodeOffset < sizeof(short))
        {
            memmove(&pstExternalClientSocket->m_szRecvBuffer[MAX_NETHEAD_SIZE], pszCodeBuffer,
                pstExternalClientSocket->m_uiRecvEndOffset - ushCodeOffset);
            pstExternalClientSocket->m_uiRecvEndOffset = MAX_NETHEAD_SIZE + pstExternalClientSocket->m_uiRecvEndOffset - ushCodeOffset;
            break;
        }

        ushCodeLength = ntohs(*(short*)pszCodeBuffer);

        //Code长度为0? 断掉这个连接
        if(ushCodeLength == 0)
        {
            return -2;
        }

        //Code长度过长,也断掉连接
        if(ushCodeLength > MAX_VALIDCODE_LENGTH || ushCodeLength > EXTERNALCLIENT_RECVBUFFER_SIZE - MAX_NETHEAD_SIZE)
        {
            return -3;
        }

        //要处理的数据包数据不够，需要接受更多的数据，所以先将其移动到MAX_NETHEAD_SIZE，继续接受数据
        if(ushCodeLength + ushCodeOffset > pstExternalClientSocket->m_uiRecvEndOffset)
        {
            memmove(&pstExternalClientSocket->m_szRecvBuffer[MAX_NETHEAD_SIZE], pszCodeBuffer,
                pstExternalClientSocket->m_uiRecvEndOffset - ushCodeOffset);
            pstExternalClientSocket->m_uiRecvEndOffset = MAX_NETHEAD_SIZE + pstExternalClientSocket->m_uiRecvEndOffset - ushCodeOffset;
            break;
        }

        iRet = m_stCodeDispatcher.DispatchOneCode(pstExternalClientSocket->m_szRecvBuffer, ushCodeOffset, ushCodeLength, iFD);

        ushCodeOffset += ushCodeLength;
        pszCodeBuffer += ushCodeLength;

        LotusStatistic::instance()->AddInputCodeNumber(1);
    }

    return 0;
}

int CIOHandler::DispatchFromInternalServer(int iSrcFD)
{
    // 从TInternalServerSocket获得NetHeadVer
    TInternalServerSocket *pstInternalServerSocket = m_pstFDPool->GetInternalSocketByFD(iSrcFD);
    if (!pstInternalServerSocket)
    {
        return -1;
    }

    unsigned short ushCodeLength = 0;
    unsigned int uiCodeOffset = 0;
    char* pszCodeBuffer = &pstInternalServerSocket->m_szRecvBuffer[0];
    int iRet = 0;

    while(true)
    {
        if(pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset < sizeof(short))
        {
            memmove(&pstInternalServerSocket->m_szRecvBuffer[0], pszCodeBuffer,
                pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset);
            pstInternalServerSocket->m_uiRecvEndOffset = pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset;
            break;
        }

        ushCodeLength = ntohs(*(short*)pszCodeBuffer);

        if(ushCodeLength == 0)
        {
            TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::DispatchFromInternalServer, Recv Codelength is 0, RecvEndOffset:%u, CodeOffset:%hd\n",
                pstInternalServerSocket->m_uiRecvEndOffset, uiCodeOffset);
            if (pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset > 0)
            {
                TRACEBIN(LOG_LEVEL_WARNING, pszCodeBuffer, pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset);
            }

            return -2;
        }

        if(ushCodeLength < PROXYHEAD_BASELENGTH || ushCodeLength > MAX_VALIDCODE_LENGTH)
        {
            TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::DispatchFromInternalServer,(%d,%d) Error CodeLength:%u, Offset = %u\n",
                pstInternalServerSocket->m_ushServerType, pstInternalServerSocket->m_ushServerID,
                ushCodeLength, pstInternalServerSocket->m_uiRecvEndOffset);
            return -3;
        }

        //移动缓冲区
        if(uiCodeOffset + ushCodeLength > pstInternalServerSocket->m_uiRecvEndOffset)
        {
            memmove(&pstInternalServerSocket->m_szRecvBuffer[0], pszCodeBuffer,
                pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset);
            pstInternalServerSocket->m_uiRecvEndOffset = pstInternalServerSocket->m_uiRecvEndOffset - uiCodeOffset;
            break;
        }

        iRet = SendToExternalClient(ushCodeLength - sizeof(short), pszCodeBuffer + sizeof(short));

        pszCodeBuffer += ushCodeLength;
        uiCodeOffset += ushCodeLength;
    }

    return 0;
}

int CIOHandler::NotifyInternalMsg(int iFD, unsigned int uiUin, unsigned short ushCtrlFlag)
{
    static char szMaxNetHeadBuffer[EXTERNALCLIENT_RECVBUFFER_SIZE];

    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
    if(!pstExternalClientSocket)
    {
        return -1;
    }

    //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::NotifyOffline, FD:%d Uin:%u\n", iFD, pstExternalClientSocket->m_uiUin);

    int iRet = m_stCodeDispatcher.DispatchToDefault(szMaxNetHeadBuffer, MAX_NETHEAD_SIZE, 0, iFD);
    if(iRet < 0)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::NotifyOffline, DispatchToCodeQueue Failed, Ret:%d, Uin:%u\n", iRet, uiUin);
    }

    return iRet;
}

int CIOHandler::Reset(int iFD, int iErrorNo, bool bNotSendOffline /* = false */)
{
    if(iFD < 0)
    {
        return -1;
    }

    //如果是外部连接，断线的时候要发个断线包到CodeQueue
    if(m_pstFDPool->IsConnectedByExternalClient(iFD))
    {
        m_stSafegurad.DecreaseExternalConnectNum();
        m_stSafegurad.IncreaseCloseNewConnectNum();

        TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);

        // add reset log
        if (NULL != pstExternalClientSocket)
        {
            char szIP[32] = {0};
            sockaddr_in stSockAddr;
            stSockAddr.sin_addr.s_addr = pstExternalClientSocket->m_uiSrcIP;
            stSockAddr.sin_port = 0;
            memcpy(szIP, inet_ntoa(stSockAddr.sin_addr), sizeof(szIP));

            time_t tNow;
            time(&tNow);
            TStruTime stTempNow;
            GetStructTime(tNow, &stTempNow);
            char szCurDateTime[32] = {0};
            snprintf(szCurDateTime, sizeof(szCurDateTime), "%04d%02d%02d%02d%02d%02d", stTempNow.m_iYear, stTempNow.m_iMon, stTempNow.m_iDay, stTempNow.m_iHour, stTempNow.m_iMin, stTempNow.m_iSec);

            TRACESVR(LOG_LEVEL_ALERT, "Reset:ErrorNo=%d,CurDateTime=%s,uin=%u,fd=%d,createTime=%u,ip=%s,port=%d,Errno=%d,ErrStr=%s\n",
                iErrorNo, szCurDateTime, pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                pstExternalClientSocket->m_uiCreateTime, szIP, pstExternalClientSocket->m_ushSrcPort, errno, strerror(errno));
        }
		
		// 去掉了条件 pstExternalClientSocket->m_iSendFlag == ESCF_SENT_PACKET_TO_CLIENT
        // 觉得这个条件会导致Server没响应的情况下, lotus不会向其发送断线通知
        if(pstExternalClientSocket != NULL && !bNotSendOffline)
        {
            //通知到后台Server
            //通过一个纯NetHead报文通知到逻辑Server
            NotifyInternalMsg(iFD, pstExternalClientSocket->m_uiUin, ERCF_LOTUS_NOTIFY_CLIENT_OFFLINE);
        }
    }

    if(iFD < (int)FD_SIZE)
    {
        m_pstFDPool->SetFDInactive(iFD);
        m_stEpollWrapper.EpollDelete(iFD);
    }

    //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::Reset, FD:%d\n", iFD);

    return CSocketOperator::Close(iFD);
}

int CIOHandler::OnAccept(unsigned int uiIP)
{
    return true;
}

int CIOHandler::CheckDefaultCodeQueue()
{
    int iRet;
    unsigned int i;
    unsigned short ushCodeLength = 0;
    char* pszCodeBufBeg = 0;
    char* pszCodeBufEnd = 0;
    char* pszHeadCode = 0;
    char* pszTailCode = 0;

    for(i = 0; i < 10000; ++i)
    {
        ushCodeLength = 0;
        iRet = m_stCodeQueueAssemble.PopOneDefault(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd,
            pszHeadCode, pszTailCode);
        if(iRet < 0)
        {
            return -1;
        }

        if(ushCodeLength == 0)
        {
            return -2;
        }

        iRet = SendToExternalClient(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd, pszHeadCode, pszTailCode);
        if(iRet < 0)
        {
            return -3;
        }
    }

    LotusStatistic::instance()->SetMaxPopedCodeNumber(i);

    return 0;
}

int CIOHandler::CheckCodeQueue()
{
#ifndef ENABLE_EXTERNAL_OUTPUT_THREAD
    // 处理从LogicServer的CodeQueue获取的消息
    CheckDefaultCodeQueue();
#endif

    return 0;
}

int CIOHandler::SendToExternalClient(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
                                     const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode)
{
    if(ushCodeLength < MIN_NETHEAD_SIZE)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, CodeLength:%d is not valid\n", ushCodeLength);
        return -1;
    }

    if(pszCodeBufBeg == 0 || pszCodeBufEnd == 0 ||
        pszHeadCode == 0 || pszTailCode == 0 || pszTailCode <= pszHeadCode)
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, invalid pointer!\n");
        return -2;
    }

    // 批量发送
    if (*(unsigned int*)pszCodeBufBeg == 0xFFFFFFFF)
    {
        return SendToExternalClientList(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd, pszHeadCode, pszTailCode);
    }

    int iFD = 0;
    TExternalClientSocket* pstExternalClientSocket = NULL;

    //如果消息没有跨过了队尾
    if(pszCodeBufEnd > pszCodeBufBeg)
    {
        //根据Uin、FD查找Socket，然后发出去
        iFD = ntohl((*(TNetHead_V2*)pszCodeBufBeg).m_uiSocketFD);

       /* TRACESVR(LOG_LEVEL_DEBUG, "In CIOHandler::SendToExternalClient, FD:%d CodeLength:%d\n", iFD, ushCodeLength - NETHEAD_V2_SIZE);
        TRACESVR(LOG_LEVEL_DEBUG, "In CIOHandler::SendToExternalClient, NetHead and Code:\n");
        TRACEBIN(LOG_LEVEL_DEBUG, (char*)pszCodeBufBeg, ushCodeLength);*/
        //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, FD:%d CodeLength:%d\n", iFD, ushCodeLength - NETHEAD_V2_SIZE);
        //TRACESVR( "In CIOHandler::SendToExternalClient, NetHead and Code:\n");

        //TRACESVR(LOG_LEVEL_DETAIL, "Msg length: %d, Cmd: %d \n",ntohl(*(int*)(pszCodeBufBeg+26)), ntohl(*(int*)(pszCodeBufBeg+30)) );


        if(iFD <= 0 || iFD >= (int)FD_SIZE)
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error FD:%d CodeLength:%d\n", iFD, ushCodeLength - NETHEAD_V2_SIZE);
            return -3;
        }

        pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
        if(!pstExternalClientSocket
          || pstExternalClientSocket->m_iSocketFD < 0)
        {
            // pstExternalClientSocket->m_iSocketFD==-1 在连接已断后，codequeue里面还有数据包下发

            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Get TExternalClientSocket by FD %d Failed.\n", iFD);
            return -4;
        }

        if(PLAYERNOTIFYTCPCLOSESOCKET == (int)(*(TNetHead_V2*)pszCodeBufBeg).m_ushSrcPort &&
            pstExternalClientSocket->m_uiUin == (*(TNetHead_V2*)pszCodeBufBeg).m_uiCodeFlag &&
            pstExternalClientSocket->m_uiCreateTime_NBO == (*(TNetHead_V2*)pszCodeBufBeg).m_uiSocketTime)
        {
            //TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET\n");
            //TRACEPLAYER(pstExternalClientSocket->m_uiUin, "In CIOHandler::SendToExternalClient, Svr Notify %u Close Socket!\n",
            //  pstExternalClientSocket->m_uiUin);
            Reset(iFD, SVR_NOTIFY_CLOSE_FD, true);
            return 0;
        }

        if(pstExternalClientSocket->m_uiCreateTime_NBO != (*(TNetHead_V2*)pszCodeBufBeg).m_uiSocketTime ||
            pstExternalClientSocket->m_uiUin != (*(TNetHead_V2*)pszCodeBufBeg).m_uiCodeFlag)
        {
            //回的包的用户已经登出了
            //TRACESVR(LOG_LEVEL_ERROR, "In CIOHandler::SendToExternalClient, FD:%d CreateTime:%u %u Uin:%u %u\n",
            //  iFD, ntohl(pstExternalClientSocket->m_uiCreateTime_NBO), ntohl((*(TNetHead_V2*)pszCodeBufBeg).m_uiSocketTime),
            //  pstExternalClientSocket->m_uiUin, (*(TNetHead_V2*)pszCodeBufBeg).m_uiCodeFlag);

            return 0;
        }

        //标志Server已经给这个客户端回包了
        pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
        pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

        int iRet = 0;

        iRet = CSocketOperator::Send(iFD, ushCodeLength - NETHEAD_V2_SIZE, pszCodeBufBeg + NETHEAD_V2_SIZE);

        if(iRet < 0 || iRet != (int)(ushCodeLength - NETHEAD_V2_SIZE))
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error Send iRet:%d\n", iRet);
            TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,sendFd=%u,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,ushCodeLength=%d,NETHEAD_V2_SIZE=%u\n", __LINE__,
                iFD,
                pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, ushCodeLength, NETHEAD_V2_SIZE);
            Reset(iFD, ERROR_EXTERAL_SEND_TO_CLIENT_FAIL);
            return 0;
        }

        LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
    }
    else
    {
        //TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::SendToExternalClient, god, we now get trouble.\n");

        if(ushCodeLength != pszTailCode - pszCodeBufBeg + (pszCodeBufEnd - pszHeadCode))
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, CodeLength not matched, ushCodeLength = %d!\n",
            //  ushCodeLength);
            return -5;
        }

        int iFirstBufferLength = pszTailCode - pszCodeBufBeg;

        //TRACESVR(LOG_LEVEL_DEBUG, "In CIOHandler::SendToExternalClient, FD:%d CodeLength:%d\n", iFD, ushCodeLength - NETHEAD_V2_SIZE);
        //TRACESVR(LOG_LEVEL_DEBUG, "In CIOHandler::SendToExternalClient, NetHead and Code:\n");
        //TRACEBIN(LOG_LEVEL_DEBUG, (char*)pszCodeBufBeg, iFirstBufferLength);
        //TRACEBIN(LOG_LEVEL_DEBUG, (char*)pszHeadCode, pszCodeBufEnd-pszHeadCode);

        if(iFirstBufferLength >= (int)sizeof(int))
        {
            iFD = ntohl(*(long*)pszCodeBufBeg);
        }
        else
        {
            char szFDBuf[4];
            memcpy(szFDBuf, pszCodeBufBeg, iFirstBufferLength);
            memcpy(szFDBuf+iFirstBufferLength, pszHeadCode, sizeof(int)-iFirstBufferLength);
            iFD = ntohl(*(long*)szFDBuf);
        }

        if(iFD <= 0 || iFD >= (int)FD_SIZE)
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error FD:%d CodeLength:%d\n",
            //  iFD, ushCodeLength - NETHEAD_V2_SIZE);
            return -6;
        }

        unsigned short ushPort = 0;
        char szPortBuf[2];

        //在解码Port前面有3个int，如果以后改变TNetHead_V2结构，注意这里要改!!!
        if((unsigned int)iFirstBufferLength <= 3 * sizeof(int))
        {
            memcpy(szPortBuf, pszHeadCode+3*sizeof(int)-iFirstBufferLength, sizeof(short));
            ushPort = *(unsigned short*)szPortBuf;
        }
        else
        {
            if((unsigned int)iFirstBufferLength >= 3 * sizeof(int) + sizeof(short))
            {
                memcpy(szPortBuf, pszCodeBufBeg+3*sizeof(int), sizeof(short));
                ushPort = *(unsigned short*)szPortBuf;
            }
            else
            {
                memcpy(szPortBuf, pszCodeBufBeg+3*sizeof(int), iFirstBufferLength-3*sizeof(int));
                memcpy(szPortBuf+iFirstBufferLength-3*sizeof(int), pszHeadCode, sizeof(short)-(iFirstBufferLength-3*sizeof(int)));
                ushPort = *(unsigned short*)szPortBuf;
            }
        }

        unsigned int uiSocketTime = 0;
        char szSocketTime[4];

        if((unsigned int)iFirstBufferLength <= sizeof(int))
        {
            memcpy(szSocketTime, pszHeadCode+sizeof(int)-iFirstBufferLength, sizeof(int));
            uiSocketTime = (unsigned int)ntohl(*(long*)szSocketTime);
        }
        else
        {
            if((unsigned int)iFirstBufferLength >= 2 * sizeof(int))
            {
                memcpy(szSocketTime, pszCodeBufBeg+sizeof(int), sizeof(int));
                uiSocketTime = (unsigned int)ntohl(*(long*)szSocketTime);
            }
            else
            {
                memcpy(szSocketTime, pszCodeBufBeg+sizeof(int), iFirstBufferLength-sizeof(int));
                memcpy(szSocketTime+iFirstBufferLength-sizeof(int), pszHeadCode, sizeof(int)-(iFirstBufferLength-sizeof(int)));
                uiSocketTime = (unsigned int)ntohl(*(long*)szSocketTime);
            }
        }

        pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
        if(!pstExternalClientSocket)
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Get TExternalClientSocket by FD %d Failed.\n", iFD);

            return -7;
        }

        if((unsigned int)PLAYERNOTIFYTCPCLOSESOCKET == ushPort && pstExternalClientSocket->m_uiCreateTime == uiSocketTime)
        {
            //TRACESVR(LOG_LEVEL_INFO, "In CIOHandler::SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET\n");
            Reset(iFD, ERROR_PLAYERNOTIFYTCPCLOSESOCKET, true);
            return 0;
        }


        if(pstExternalClientSocket->m_uiCreateTime != uiSocketTime)
        {
            //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, FD:%d CreateTime:%u %u\n", iFD,
            //  pstExternalClientSocket->m_uiCreateTime, uiSocketTime);

            return 0;
        }

        pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
        pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

        if((unsigned int)iFirstBufferLength > NETHEAD_V2_SIZE)
        {
            int iRet = 0;
            int iSecondSendLen = pszCodeBufEnd - pszHeadCode;

            iRet = CSocketOperator::Send(iFD, iFirstBufferLength - NETHEAD_V2_SIZE, pszCodeBufBeg + NETHEAD_V2_SIZE);

            if(iRet < 0 || iRet != (int)(iFirstBufferLength - NETHEAD_V2_SIZE))
            {
                //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error Send iRet:%d\n", iRet);
                TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iFirstBufferLength=%d,NETHEAD_V2_SIZE=%u\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iFirstBufferLength, NETHEAD_V2_SIZE);
                Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_1);
                return 0;
            }

            iRet = CSocketOperator::Send(iFD, iSecondSendLen, pszHeadCode);
            if(iRet < 0 || iRet != iSecondSendLen)
            {
                //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error Send iRet:%d\n", iRet);
                TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iSecondSendLen=%d\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iSecondSendLen);
                Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_2);
                return 0;
            }

            LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
        }
        else
        {
            int iRet = 0;
            int iTotalSendLen = pszCodeBufEnd - pszHeadCode - (NETHEAD_V2_SIZE - iFirstBufferLength);

            iRet = CSocketOperator::Send(iFD, iTotalSendLen, pszHeadCode + NETHEAD_V2_SIZE - iFirstBufferLength);

            if(iRet < 0 || iRet != iTotalSendLen)
            {
                //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error Send iRet:%d\n", iRet);
                TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iTotalSendLen=%d\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iTotalSendLen);
                Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_3);
                return 0;
            }

            LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
        }
    }

    //TRACEPLAYER(pstExternalClientSocket->m_uiUin, "In CIOHandler::SendToExternalClient, CodeLength = %d\n",
    //    ushCodeLength - NETHEAD_V2_SIZE);

    m_stSafegurad.IncreaseDownPacketNum();

    //TRACESVR(LOG_LEVEL_NOTICE, "In CIOHandler::SendToExternalClient, FD:%d Send OK\n", iFD);

    return 0;
}

int CIOHandler::SendToAllExternalClient(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
                                        const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode)
{
    if(ushCodeLength < MIN_NETHEAD_SIZE)
    {
        return -1;
    }

    if(pszCodeBufBeg == 0 || pszCodeBufEnd == 0 ||
        pszHeadCode == 0 || pszTailCode == 0 || pszTailCode <= pszHeadCode)
    {
        return -2;
    }

    int iNetHeadUinListSize = 2 * sizeof(unsigned int);

    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalFirstSocket();
    TExternalClientSocket* pstNextExternalClientSocket = m_pstFDPool->GetExternalFirstSocket();

    //如果消息没有跨过了队尾
    if(pszCodeBufEnd > pszCodeBufBeg)
    {
        while (pstExternalClientSocket)
        {
            pstExternalClientSocket = pstNextExternalClientSocket;
            if (!pstExternalClientSocket)
            {
                break;
            }

            pstNextExternalClientSocket = pstExternalClientSocket->m_pNextSocket;

            int iFD = pstExternalClientSocket->m_iSocketFD;

            // 当玩家下线后，OnRead->OnReadFromExternal->Reset->SetFDInactive->DeleteSocketByFD，这里已经把pstExternalClientSocket->m_iSocketFD 重置为-1
            // 然后lotus又通知zone，zone写下线消息给lotus。所以这里直接continue。否则fd=-1 往下执行
            if (iFD <= 0)
            {
                continue;
            }

            //标志Server已经给这个客户端回包了
            pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
            pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

            int iRet = 0;
            iRet = CSocketOperator::Send(iFD, ushCodeLength - iNetHeadUinListSize, pszCodeBufBeg + iNetHeadUinListSize);

            if(iRet < 0 || iRet != (int)(ushCodeLength - iNetHeadUinListSize))
            {
                TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,ushCodeLength=%d,iNetHeadUinListSize=%d\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, ushCodeLength, iNetHeadUinListSize);
                Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_4);
                continue;
            }

            LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
        }
    }
    else
    {
        if(ushCodeLength != pszTailCode - pszCodeBufBeg + (pszCodeBufEnd - pszHeadCode))
        {
            return -5;
        }

        int iFirstBufferLength = pszTailCode - pszCodeBufBeg;

        while (pstExternalClientSocket)
        {
            pstExternalClientSocket = pstNextExternalClientSocket;
            if (!pstExternalClientSocket)
            {
                break;
            }

            pstNextExternalClientSocket = pstExternalClientSocket->m_pNextSocket;

            //标志Server已经给这个客户端回包了
            pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
            pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

            int iFD = pstExternalClientSocket->m_iSocketFD;

            // 当玩家下线后，OnRead->OnReadFromExternal->Reset->SetFDInactive->DeleteSocketByFD，这里已经把pstExternalClientSocket->m_iSocketFD 重置为-1
            // 然后lotus又通知zone，zone写下线消息给lotus。所以这里直接continue。否则fd=-1 往下执行
            if (iFD <= 0)
            {
                continue;
            }

            if(iFirstBufferLength > iNetHeadUinListSize)
            {
                int iRet = 0;
                int iSecondSendLen = pszCodeBufEnd - pszHeadCode;

                iRet = CSocketOperator::Send(iFD, iFirstBufferLength - iNetHeadUinListSize, pszCodeBufBeg + iNetHeadUinListSize);

                if(iRet < 0 || iRet != (int)(iFirstBufferLength - iNetHeadUinListSize))
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d, uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iFirstBufferLength=%d,iNetHeadUinListSize=%d\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iFirstBufferLength, iNetHeadUinListSize);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_5);
                    continue;
                }

                iRet = CSocketOperator::Send(iFD, iSecondSendLen, pszHeadCode);
                if(iRet < 0 || iRet != iSecondSendLen)
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iSecondSendLen=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iSecondSendLen);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_6);
                    continue;
                }

                LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
            }
            else
            {
                int iRet = 0;
                int iTotalSendLen = pszCodeBufEnd - pszHeadCode - (iNetHeadUinListSize - iFirstBufferLength);

                iRet = CSocketOperator::Send(iFD, iTotalSendLen, pszHeadCode + iNetHeadUinListSize - iFirstBufferLength);

                if(iRet < 0 || iRet != iTotalSendLen)
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iTotalSendLen=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iTotalSendLen);

                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_7);
                    continue;
                }

                LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
            }
        }
    }

    return 0;
}

int CIOHandler::SendToAllExternalClientButOne(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
                                              const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode)
{
    if(ushCodeLength < MIN_NETHEAD_SIZE)
    {
        return -1;
    }

    if(pszCodeBufBeg == 0 || pszCodeBufEnd == 0 ||
        pszHeadCode == 0 || pszTailCode == 0 || pszTailCode <= pszHeadCode)
    {
        return -2;
    }

    int iNetHeadUinListSize = 3 * sizeof(unsigned int);

    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalFirstSocket();
    TExternalClientSocket* pstNextExternalClientSocket = m_pstFDPool->GetExternalFirstSocket();

    //如果消息没有跨过了队尾
    if(pszCodeBufEnd > pszCodeBufBeg)
    {
        unsigned int* pSocketFD = (unsigned int*)(pszCodeBufBeg + 2*sizeof(unsigned int));
        int iExFD = ntohl(pSocketFD[0]);

        while (pstExternalClientSocket)
        {
            pstExternalClientSocket = pstNextExternalClientSocket;
            if (!pstExternalClientSocket)
            {
                break;
            }

            pstNextExternalClientSocket = pstExternalClientSocket->m_pNextSocket;

            int iFD = pstExternalClientSocket->m_iSocketFD;
            if (iFD == iExFD
                || iFD < 0)
            {
                continue;
            }

            //标志Server已经给这个客户端回包了
            pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
            pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

            int iRet = 0;
            iRet = CSocketOperator::Send(iFD, ushCodeLength - iNetHeadUinListSize, pszCodeBufBeg + iNetHeadUinListSize);

            if(iRet < 0 || iRet != (int)(ushCodeLength - iNetHeadUinListSize))
            {
                TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,ushCodeLength=%d,iNetHeadUinListSize=%d\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, ushCodeLength, iNetHeadUinListSize);
                Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_8);
                continue;
            }

            LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
        }
    }
    else
    {
        if(ushCodeLength != pszTailCode - pszCodeBufBeg + (pszCodeBufEnd - pszHeadCode))
        {
            return -5;
        }

        int iFirstBufferLength = pszTailCode - pszCodeBufBeg;

        // 解析SocketFD
        const int MAX_SOCKETFD_NUMBER = 200;
        char szTmpBuf[(2 + MAX_SOCKETFD_NUMBER) * sizeof(unsigned int)];
        unsigned int* pSocketFD = (unsigned int*)(&szTmpBuf[2*sizeof(unsigned int)]);
        if (iFirstBufferLength >= iNetHeadUinListSize)
        {
            pSocketFD = (unsigned int*)(pszCodeBufBeg + 2* sizeof(unsigned int));
        }
        else
        {
            memcpy(szTmpBuf, pszCodeBufBeg, iFirstBufferLength);
            memcpy(szTmpBuf+iFirstBufferLength, pszHeadCode, iNetHeadUinListSize -iFirstBufferLength);
        }

        int iExFD = ntohl(pSocketFD[0]);

        while (pstExternalClientSocket)
        {
            pstExternalClientSocket = pstNextExternalClientSocket;
            if (!pstExternalClientSocket)
            {
                break;
            }

            pstNextExternalClientSocket = pstExternalClientSocket->m_pNextSocket;
            int iFD = pstExternalClientSocket->m_iSocketFD;
            if (iFD == iExFD)
            {
                continue;
            }

            //标志Server已经给这个客户端回包了
            pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
            pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

            if(iFirstBufferLength > iNetHeadUinListSize)
            {
                int iRet = 0;
                int iSecondSendLen = pszCodeBufEnd - pszHeadCode;

                iRet = CSocketOperator::Send(iFD, iFirstBufferLength - iNetHeadUinListSize, pszCodeBufBeg + iNetHeadUinListSize);

                if(iRet < 0 || iRet != (int)(iFirstBufferLength - iNetHeadUinListSize))
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iFirstBufferLength=%d,iNetHeadUinListSize=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iFirstBufferLength, iNetHeadUinListSize);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_9);
                    continue;
                }

                iRet = CSocketOperator::Send(iFD, iSecondSendLen, pszHeadCode);
                if(iRet < 0 || iRet != iSecondSendLen)
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iSecondSendLen=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iSecondSendLen);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_10);
                    continue;
                }

                LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
            }
            else
            {
                int iRet = 0;
                int iTotalSendLen = pszCodeBufEnd - pszHeadCode - (iNetHeadUinListSize - iFirstBufferLength);

                iRet = CSocketOperator::Send(iFD, iTotalSendLen, pszHeadCode + iNetHeadUinListSize - iFirstBufferLength);

                if(iRet < 0 || iRet != iTotalSendLen)
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iTotalSendLen=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iTotalSendLen);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_11);
                    continue;
                }

                LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
            }
        }
    }

    return 0;
}

// codequeue uinlist
int CIOHandler::SendToExternalClientList(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
                                         const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode)
{
    if(ushCodeLength < MIN_NETHEAD_SIZE)
    {
        return -1;
    }

    if(pszCodeBufBeg == 0 || pszCodeBufEnd == 0 ||
        pszHeadCode == 0 || pszTailCode == 0 || pszTailCode <= pszHeadCode)
    {
        return -2;
    }

    unsigned int uiSocketNumber = 0;
    int iNetHeadUinListSize = 0;
    TExternalClientSocket* pstExternalClientSocket = NULL;

    //如果消息没有跨过了队尾
    if(pszCodeBufEnd > pszCodeBufBeg)
    {
        uiSocketNumber = *(unsigned int*)(pszCodeBufBeg + sizeof(unsigned int));
        if (uiSocketNumber == SEND_ALL_SESSION)
        {
            return SendToAllExternalClient(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd, pszHeadCode, pszTailCode);
        }
        else if (uiSocketNumber == SEND_ALL_SESSION_BUTONE)
        {
            return SendToAllExternalClientButOne(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd, pszHeadCode, pszTailCode);
        }

        iNetHeadUinListSize = (uiSocketNumber + 2) * sizeof(unsigned int);
        unsigned int* pSocketFD = (unsigned int*)(pszCodeBufBeg + 2*sizeof(unsigned int));

        for (int i = 0; i < (int)uiSocketNumber; i++)
        {
            //根据Uin、FD查找Socket，然后发出去
            int iFD = ntohl(pSocketFD[i]);
            if(iFD <= 0 || iFD >= (int)FD_SIZE)
            {
                continue;
            }

            pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
            if(!pstExternalClientSocket
                || pstExternalClientSocket->m_iSocketFD < 0)
            {
                continue;
            }

            //标志Server已经给这个客户端回包了
            pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
            pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

            int iRet = 0;

            iRet = CSocketOperator::Send(iFD, ushCodeLength - iNetHeadUinListSize, pszCodeBufBeg + iNetHeadUinListSize);

            if(iRet < 0 || iRet != (int)(ushCodeLength - iNetHeadUinListSize))
            {
                TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,ushCodeLength=%d,iNetHeadUinListSize=%d\n", __LINE__,
                    pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                    pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, ushCodeLength, iNetHeadUinListSize);
                Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_12);
                continue;
            }

            LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
        }
    }
    else
    {
        if(ushCodeLength != pszTailCode - pszCodeBufBeg + (pszCodeBufEnd - pszHeadCode))
        {
            return -5;
        }

        int iFirstBufferLength = pszTailCode - pszCodeBufBeg;

        // 解析SocketNumber
        if(iFirstBufferLength >= 2 * (int)sizeof(int))
        {
            uiSocketNumber = *(unsigned int*)(pszCodeBufBeg[sizeof(unsigned int)]);
        }
        else
        {
            char szTmpBuf[2*sizeof(unsigned int)];
            memcpy(szTmpBuf, pszCodeBufBeg, iFirstBufferLength);
            memcpy(szTmpBuf+iFirstBufferLength, pszHeadCode, 2* sizeof(unsigned int) -iFirstBufferLength);
            uiSocketNumber = *(unsigned int*)(szTmpBuf[sizeof(unsigned int)]);
        }

        if (uiSocketNumber == SEND_ALL_SESSION)
        {
            return SendToAllExternalClient(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd, pszHeadCode, pszTailCode);
        }
        else if (uiSocketNumber == SEND_ALL_SESSION_BUTONE)
        {
            return SendToAllExternalClientButOne(ushCodeLength, pszCodeBufBeg, pszCodeBufEnd, pszHeadCode, pszTailCode);
        }

        iNetHeadUinListSize = (uiSocketNumber + 2) * sizeof(unsigned int);

        // 解析SocketFD
        const int MAX_SOCKETFD_NUMBER = 200;
        char szTmpBuf[(2 + MAX_SOCKETFD_NUMBER) * sizeof(unsigned int)];
        unsigned int* pSocketFD = (unsigned int*)(&szTmpBuf[2*sizeof(unsigned int)]);
        if (iFirstBufferLength >= iNetHeadUinListSize)
        {
            pSocketFD = (unsigned int*)(pszCodeBufBeg + 2* sizeof(unsigned int));
        }
        else
        {
            memcpy(szTmpBuf, pszCodeBufBeg, iFirstBufferLength);
            memcpy(szTmpBuf+iFirstBufferLength, pszHeadCode, iNetHeadUinListSize -iFirstBufferLength);
        }

        for (int i = 0; i < (int)uiSocketNumber; i++)
        {
            int iFD = ntohl(pSocketFD[i]);
            if(iFD <= 0 || iFD >= (int)FD_SIZE)
            {
                return -6;
            }

            pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
            if(!pstExternalClientSocket)
            {
                continue;
            }

            //标志Server已经给这个客户端回包了
            pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
            pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

            if(iFirstBufferLength > iNetHeadUinListSize)
            {
                int iRet = 0;
                int iSecondSendLen = pszCodeBufEnd - pszHeadCode;

                iRet = CSocketOperator::Send(iFD, iFirstBufferLength - iNetHeadUinListSize, pszCodeBufBeg + iNetHeadUinListSize);

                if(iRet < 0 || iRet != (int)(iFirstBufferLength - iNetHeadUinListSize))
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iFirstBufferLength=%d,iNetHeadUinListSize=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iFirstBufferLength, iNetHeadUinListSize);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_13);
                    continue;
                }

                iRet = CSocketOperator::Send(iFD, iSecondSendLen, pszHeadCode);
                if(iRet < 0 || iRet != iSecondSendLen)
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iSecondSendLen=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iSecondSendLen);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_14);
                    continue;
                }

                LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
            }
            else
            {
                int iRet = 0;
                int iTotalSendLen = pszCodeBufEnd - pszHeadCode - (iNetHeadUinListSize - iFirstBufferLength);

                iRet = CSocketOperator::Send(iFD, iTotalSendLen, pszHeadCode + iNetHeadUinListSize - iFirstBufferLength);

                if(iRet < 0 || iRet != iTotalSendLen)
                {
                    TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,iTotalSendLen=%d\n", __LINE__,
                        pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
                        pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, iTotalSendLen);
                    Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_15);
                    continue;
                }

                LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
            }
        }
    }

    m_stSafegurad.IncreaseDownPacketNum();

    return 0;
}

int CIOHandler::SendToExternalClient(const unsigned short ushCodeLength, const char* pszCodeBuffer)
{
    if(ushCodeLength < MIN_NETHEAD_SIZE)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, CodeLength:%d is not valid\n", ushCodeLength);
        return -1;
    }

    //根据Uin、FD查找Socket，然后发出去
    int iFD = 0;
    int iNetHeadLength = 0;
    unsigned short ushSrcPort = 0;
    unsigned int uiSocketTime = 0;
    unsigned int uiUin = 0;



    TRACESVR(LOG_LEVEL_DEBUG, "In CIOHandler::SendToExternalClient, FD:%d CodeLength:%d\n", iFD, ushCodeLength - iNetHeadLength);
    TRACESVR(LOG_LEVEL_DEBUG, "In CIOHandler::SendToExternalClient, NetHead and Code:\n");
    TRACEBIN(LOG_LEVEL_DEBUG, (char*)pszCodeBuffer, ushCodeLength);

    if(iFD <= 0 || iFD >= (int)FD_SIZE)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error FD:%d CodeLength:%d\n",
            iFD, ushCodeLength - iNetHeadLength);
        return -2;
    }

    if(PLAYERNOTIFYTCPCLOSESOCKET == ushSrcPort)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET\n");
        Reset(iFD, PLAYERNOTIFYTCPCLOSESOCKET_2);
        return 0;
    }

    TExternalClientSocket* pstExternalClientSocket = m_pstFDPool->GetExternalSocketByFD(iFD);
    if(!pstExternalClientSocket)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, pstExternalClientSocket=NULL\n");
        return -3;
    }

    bool bCheckNetHead  = (ntohl(pstExternalClientSocket->m_uiCreateTime_NBO) == uiSocketTime
            && uiUin == pstExternalClientSocket->m_uiUin);

    //不需要检查,认为检查成功
    if(!m_stConfigAssemble.GetLotusConfigMng().IsNeedCheckNetHead())
    {
        bCheckNetHead = true;
    }

    if(!bCheckNetHead)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, UnMatch Time or Uin, FD:%d CreateTime:%u %u Uin:%u %u\n",
            iFD, ntohl(pstExternalClientSocket->m_uiCreateTime_NBO), uiSocketTime,
            pstExternalClientSocket->m_uiUin, uiUin);
        return 0;
    }
    else
    {
        TRACESVR(LOG_LEVEL_DETAIL,
            "In CIOHandler::SendToExternalClient, Match Time or Uin, FD:%d CreateTime:%u %u Uin:%u %u\n",
            iFD, ntohl(pstExternalClientSocket->m_uiCreateTime_NBO), uiSocketTime,
            pstExternalClientSocket->m_uiUin, uiUin);
    }

    pstExternalClientSocket->m_iSendFlag = ESCF_SENT_PACKET_TO_CLIENT;
    pstExternalClientSocket->m_uiRecvSvrTimeStamp = time(0);

    int iRet = 0;
    iRet = CSocketOperator::Send(iFD, ushCodeLength - iNetHeadLength, pszCodeBuffer + iNetHeadLength);

    if(iRet < 0 || iRet != (int)(ushCodeLength - iNetHeadLength))
    {
        //TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Error Send iRet:%d\n", iRet);
        TRACESVR(LOG_LEVEL_ALERT, "Send Err:line=%d,uin=%u,fd=%d,createTime=%u,Errno=%d,ErrStr=%s,iRet=%d,ushCodeLength=%d,iNetHeadLength=%d\n", __LINE__,
            pstExternalClientSocket->m_uiUin, pstExternalClientSocket->m_iSocketFD,
            pstExternalClientSocket->m_uiCreateTime, errno, strerror(errno), iRet, ushCodeLength, iNetHeadLength);
        Reset(iFD, ERROR_EXTERAL_SEND_CLIENT_16);
        return 0;
    }

    TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::SendToExternalClient, Succ Send iLength:%d \n", ushCodeLength - iNetHeadLength);


    TRACEPLAYER(pstExternalClientSocket->m_uiUin, "In CIOHandler::SendToExternalClient, CodeLength = %d\n",
        ushCodeLength - iNetHeadLength);

    LotusStatistic::instance()->AddOutputPacket(ushCodeLength);
    m_stSafegurad.IncreaseDownPacketNum();

    TRACESVR(LOG_LEVEL_NOTICE, "In CIOHandler::SendToExternalClient, FD:%d Send OK iRet:%d\n", iFD, iRet);

    return 0;
}
