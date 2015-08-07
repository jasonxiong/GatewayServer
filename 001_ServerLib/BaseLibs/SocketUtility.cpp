//:SocketUtility.cpp
/************************************************************
FileName: SocketUtility.cpp
Author: jasonxiong
Description:
Version: 1.0
History:
<author>    <time>    <version >    <desc>
jasonxiong   09/11/02/      1.0         create
***********************************************************/

//在这添加标准库头文件
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

//在这添加ServerLib头文件
#include "StringUtility.hpp"
#include "SocketUtility.hpp"

using namespace ServerLib;

int CSocketUtility::SetNBlock(int iSocketFD)
{
#ifdef WIN32
    u_long ulNonBlock = 1;
    ioctlsocket(iSocketFD, FIONBIO, &ulNonBlock);
#else
    int iFlags;
    iFlags = fcntl(iSocketFD, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    fcntl(iSocketFD, F_SETFL, iFlags);
#endif

    return 0;
}

int CSocketUtility::SetNagleOff(int iSocketFD)
{
    /* Disable the Nagle (TCP No Delay) algorithm */
    int iFlags = 1;

    setsockopt(iSocketFD, IPPROTO_TCP, TCP_NODELAY, (char *)&iFlags, sizeof(iFlags));
    return 0;
}

int CSocketUtility::SetSockRecvBufLen(int iSocketFD, int& riRecvBufLen)
{
    int iSetBufLen = riRecvBufLen;
    int iOptLen = sizeof(int);

    if(iSetBufLen > 0)
    {
        int iRet = setsockopt(iSocketFD, SOL_SOCKET, SO_RCVBUF, &iSetBufLen, iOptLen);

        if(iRet)
        {
            return -1;
        }
    }

    int iRet = getsockopt(iSocketFD, SOL_SOCKET, SO_RCVBUF, (void *)&iSetBufLen, (socklen_t *)&iOptLen);

    if(iRet)
    {
        return -2;
    }

    riRecvBufLen = iSetBufLen;

    return 0;
}

int CSocketUtility::SetSockSendBufLen(int iSocketFD, int& riSendBufLen)
{
    int iSetBufLen = riSendBufLen;
    int iOptLen = sizeof(int);

    if(iSetBufLen > 0)
    {
        int iRet = setsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, &iSetBufLen, iOptLen);

        if(iRet)
        {
            return -1;
        }
    }

    int iRet = getsockopt(iSocketFD, SOL_SOCKET, SO_SNDBUF, (void *)&iSetBufLen, (socklen_t *)&iOptLen);

    if(iRet)
    {
        return -2;
    }

    riSendBufLen = iSetBufLen;

    return 0;
}

int CSocketUtility::Connect(int iSocketFD, const char* pszServerIP, unsigned short ushServerPort)
{
    if(pszServerIP == NULL || pszServerIP[0] == '\0')
    {
        return -1;
    }

    sockaddr_in stTempAddr;
    memset((void *)&stTempAddr, 0, sizeof(sockaddr_in));
    stTempAddr.sin_family = AF_INET;
    stTempAddr.sin_port = htons(ushServerPort);
    stTempAddr.sin_addr.s_addr = inet_addr(pszServerIP);

    int iRet = connect(iSocketFD, (const struct sockaddr *)&stTempAddr, sizeof(stTempAddr));

    return iRet;
}

int CSocketUtility::SetReuseAddr(int iSocketFD)
{
    int iReusePortFlag = 1;

    int iRet = setsockopt(iSocketFD, SOL_SOCKET, SO_REUSEADDR, &iReusePortFlag, sizeof(int));

    return iRet;
}

int CSocketUtility::SetKeepalive(int iSocketFD)
{
    int iFlag = 1;
    setsockopt(iSocketFD, SOL_SOCKET, SO_KEEPALIVE, &iFlag, sizeof(iFlag));
    return 0;
}

int CSocketUtility::SetLingerOff(int iSocketFD)
{
    struct linger stLinger;
    stLinger.l_onoff = 0;
    stLinger.l_linger = 0;
    setsockopt(iSocketFD, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger));
    return 0;
}

int CSocketUtility::BindSocket(int iSocketFD, const char* pszBindIP, unsigned short ushBindPort)
{
    sockaddr_in stTempSockAddr;
    memset((void *)&stTempSockAddr, 0, sizeof(sockaddr_in));
    stTempSockAddr.sin_family = AF_INET;
    stTempSockAddr.sin_port = htons(ushBindPort);

    if(pszBindIP == NULL || pszBindIP[0] == '\0')
    {
        stTempSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        stTempSockAddr.sin_addr.s_addr = inet_addr(pszBindIP);
    }

    int iRet = bind(iSocketFD, (const struct sockaddr *)&stTempSockAddr, sizeof(stTempSockAddr));

    return iRet;
}

int CSocketUtility::IPInt32ToString(int iIPAddr, char* szAddr)
{
    if(szAddr == NULL)
    {
        return -1;
    }

    sockaddr_in stAddr;
    stAddr.sin_addr.s_addr = iIPAddr;
    char* pcTempIP = inet_ntoa(stAddr.sin_addr);

    if(pcTempIP == NULL)
    {
        return -2;
    }

    strcpy(szAddr, pcTempIP);

    return 0;
}

int CSocketUtility::SockAddrToString(sockaddr_in *pstSockAddr, char *szAddr)
{
    char *pcTempIP = NULL;
    unsigned short nTempPort = 0;

    if( !pstSockAddr || !szAddr )
    {
        return -1;
    }

    pcTempIP = inet_ntoa(pstSockAddr->sin_addr);

    if( !pcTempIP )
    {
        return -1;
    }

    nTempPort = ntohs(pstSockAddr->sin_port);

    sprintf(szAddr, "%s:%d", pcTempIP, nTempPort);

    return 0;
}

///:~
