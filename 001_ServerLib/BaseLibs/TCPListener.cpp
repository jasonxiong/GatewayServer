/**
*@file TCPListener.cpp
*@author jasonxiong
*@date 2009-11-19
*@version 1.0
*@brief
*
*
*/

//在这添加标准库头文件
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "TCPListener.hpp"
#include "SocketUtility.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

CTCPListener::CTCPListener()
{
    m_iSocketFD = -1;
    m_iStatus = 0;
    memset(&m_stTCPListenConfig, 0, sizeof(m_stTCPListenConfig));
    m_iErrorNO = 0;
}

CTCPListener::~CTCPListener()
{

}

int CTCPListener::Accept(int& riAcceptFD, unsigned int& ruiIP, unsigned short& rushPort)
{
    if(m_iStatus != tls_opened || m_iSocketFD < 0)
    {
        SetErrorNO(EEN_TCP_LISTENER__INVALID_FD);

        return -1;
    }

    sockaddr_in stTempSockAddr;
    int iSocketAddrSize = sizeof(stTempSockAddr);

    int iFD = accept(m_iSocketFD, (struct sockaddr *)&stTempSockAddr, (socklen_t *)&iSocketAddrSize);

    if(iFD < 0)
    {
        SetErrorNO(EEN_TCP_LISTENER__ACCEPT_FAILED);

        return -2;
    }

    riAcceptFD = iFD;
    ruiIP = stTempSockAddr.sin_addr.s_addr;
    rushPort = stTempSockAddr.sin_port;

    return 0;
}

int CTCPListener::Initialize(const char *pszBindIP, const unsigned short ushBindPort)
{
    TTCPListenerCfg stListenCfg;
    memset(&stListenCfg, 0, sizeof(stListenCfg));

    if(pszBindIP != NULL)
    {
        SAFE_STRCPY(stListenCfg.m_szBindIP, pszBindIP,
                    sizeof(stListenCfg.m_szBindIP)-1);
    }

    stListenCfg.m_ushBindPort = ushBindPort;
    stListenCfg.m_iBacklogFDNum = DEFAULT_BACKLOG_FD_NUMBER;

    return Initialize(stListenCfg);
}

int CTCPListener::Initialize(const TTCPListenerCfg& rstListenerConfig)
{
    m_stTCPListenConfig = rstListenerConfig;

    int iRet = CreateSocket();

    if(iRet)
    {
        return -1;
    }

    iRet = CSocketUtility::SetReuseAddr(m_iSocketFD);

    if(iRet)
    {
        CloseSocket();
        SetErrorNO(EEN_TCP_LISTENER__REUSE_ADDR_FAILED);

        return -2;
    }

    iRet = CSocketUtility::BindSocket(m_iSocketFD, rstListenerConfig.m_szBindIP, rstListenerConfig.m_ushBindPort);

    if(iRet)
    {
        CloseSocket();
        SetErrorNO(EEN_TCP_LISTENER__BIND_SOCKET_FAILED);

        return -3;
    }

    if(m_stTCPListenConfig.m_iBacklogFDNum < MIN_BACKLOG_FD_NUMBER ||
            m_stTCPListenConfig.m_iBacklogFDNum > MAX_BACKLOG_FD_NUMBER)
    {
        m_stTCPListenConfig.m_iBacklogFDNum = DEFAULT_BACKLOG_FD_NUMBER;
    }

    iRet = listen(m_iSocketFD, m_stTCPListenConfig.m_iBacklogFDNum);

    if(iRet)
    {
        CloseSocket();
        SetErrorNO(EEN_TCP_LISTENER__LISTEN_FAILED);

        return -4;
    }

    CSocketUtility::SetNBlock(m_iSocketFD);
    CSocketUtility::SetKeepalive(m_iSocketFD);
    CSocketUtility::SetLingerOff(m_iSocketFD);
    m_iStatus = tls_opened;

    return 0;
}

int CTCPListener::CreateSocket()
{
    if(m_iStatus != tls_closed && m_iSocketFD > 0)
    {
        CloseSocket();
    }

    m_iSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if(m_iSocketFD < 0)
    {
        SetErrorNO(EEN_TCP_LISTENER__SOCKET_FAILED);

        m_iStatus = tls_closed;
        m_iSocketFD = -1;

        return -1;
    }

    return 0;
}

int CTCPListener::CloseSocket()
{
    if(m_iSocketFD > 0)
    {
        close(m_iSocketFD);
    }

    m_iSocketFD = -1;
    m_iStatus = tls_closed;

    return 0;
}


