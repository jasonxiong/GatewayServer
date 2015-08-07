/**
*@file UDPSocket.cpp
*@author jasonxiong
*@date 2010-01-26
*@version 1.0
*@brief UDPSocket的实现文件
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
#include <pthread.h>

#include "UDPSocket.hpp"
#include "ErrorDef.hpp"
#include "SocketUtility.hpp"
#include "NowTime.hpp"
#include "SharedMemory.hpp"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"
using namespace ServerLib;

CUDPSocket::CUDPSocket()
{
    // 默认使用内部缓冲区

    m_iUsrBufAllocType = EURT_ALLOC_BY_UDP_DEFAULT;
    m_iUsrRecvBufSize = MAX_BUFFER_LENGTH;
    m_abyRecvBuffer = m_szBufferContent;
    m_iReadBegin = m_iReadEnd = 0;
    m_iSysRecvBufSize = 0;
    m_iSysSendBufSize = 0;
    memset(&m_stUDPSocketCfg, 0, sizeof(m_stUDPSocketCfg));

    m_iSocketFD = -1;
    m_iStatus = EUSS_CLOSED;
    m_iErrorNO = 0;
}

CUDPSocket::CUDPSocket(int iUsrRecvBufSize)
{
    __ASSERT_AND_LOG(iUsrRecvBufSize > 0);

    //如果用户缓冲区采用的是由CUDPSocket分配

    m_iUsrBufAllocType = EURT_ALLOC_BY_UDP_SOCKET;
    m_iUsrRecvBufSize = iUsrRecvBufSize;
    m_abyRecvBuffer = new unsigned char[m_iUsrRecvBufSize];
    m_iReadBegin = m_iReadEnd = 0;
    __ASSERT_AND_LOG(m_abyRecvBuffer);
    m_iSysRecvBufSize = 0;
    m_iSysSendBufSize = 0;
    memset(&m_stUDPSocketCfg, 0, sizeof(m_stUDPSocketCfg));

    m_iSocketFD = -1;
    m_iStatus = EUSS_CLOSED;
    m_iErrorNO = 0;
}

CUDPSocket::~CUDPSocket()
{
    CloseSocket();

    if(m_iUsrBufAllocType == EURT_ALLOC_BY_UDP_SOCKET)
    {
        FreeUsrBuffer();
    }
}

CUDPSocket* CUDPSocket::CreateBySharedMemory(const char* pszKeyFileName,
        const unsigned char ucKeyPrjID,
        int iUsrRecvBufSize)
{
    if(pszKeyFileName == NULL || iUsrRecvBufSize <= 0)
    {
        return NULL;
    }

    CSharedMemory stSharedMemory;
    unsigned int uiSharedMemorySize = sizeof(CUDPSocket) + iUsrRecvBufSize;

    int iRet = stSharedMemory.CreateShmSegment(pszKeyFileName, ucKeyPrjID,
               (int)uiSharedMemorySize);

    if(iRet)
    {
        return NULL;
    }

    //在共享内存的地址上分配CUDPSocket
    CUDPSocket* pstUDPSocket = (CUDPSocket*)stSharedMemory.GetFreeMemoryAddress();

    if(!pstUDPSocket)
    {
        return NULL;
    }

    pstUDPSocket->m_iUsrRecvBufSize = iUsrRecvBufSize;
    pstUDPSocket->m_iUsrBufAllocType = EURT_ALLOC_BY_SHARED_MEMORY;
    pstUDPSocket->m_abyRecvBuffer = (unsigned char*)stSharedMemory.GetFreeMemoryAddress() + sizeof(CUDPSocket);

    return pstUDPSocket;
}

int CUDPSocket::Initialize(const char* pszServerIP, unsigned short ushServerPort,
                           const char* pszBindIP, unsigned short ushBindPort)
{
    if(pszServerIP == NULL || ushServerPort <= 0)
    {
        SetErrorNO(EEN_UDP_SOCKET__NULL_POINTER);

        return -1;
    }

    TUDPSocketConfig stSocketCfg;
    memset(&stSocketCfg, 0, sizeof(stSocketCfg));
    SAFE_STRCPY(stSocketCfg.m_szServerIP, pszServerIP, sizeof(stSocketCfg.m_szServerIP)-1);
    stSocketCfg.m_ushServerPort = ushServerPort;

    if(pszBindIP != NULL)
    {
        SAFE_STRCPY(stSocketCfg.m_szBindIP, pszBindIP, sizeof(stSocketCfg.m_szBindIP)-1);
        stSocketCfg.m_ushBindPort = ushBindPort;
    }

    stSocketCfg.m_iSysRecvBufSize = DEFAULT_UDP_SYS_RECV_BUFFER_SIZE;
    stSocketCfg.m_iSysSendBufSize = DEFAULT_UDP_SYS_SEND_BUFFER_SIZE;

    return Initialize(stSocketCfg);
}

int CUDPSocket::Initialize(const TUDPSocketConfig& rstUDPConfig)
{
    m_stUDPSocketCfg = rstUDPConfig;

    int iRet = CreateSocket();

    if(iRet)
    {
        return -1;
    }

    if(m_iSocketFD < 0)
    {
        SetErrorNO(EEN_UDP_SOCKET__INVALID_FD);

        return -2;
    }

    //到这连接已经创建好了，需要做些后续工作
    CSocketUtility::SetNBlock(m_iSocketFD);

    m_iSysRecvBufSize = rstUDPConfig.m_iSysRecvBufSize;
    CSocketUtility::SetSockRecvBufLen(m_iSocketFD, m_iSysRecvBufSize);
    m_iSysSendBufSize = rstUDPConfig.m_iSysSendBufSize;
    CSocketUtility::SetSockSendBufLen(m_iSocketFD, m_iSysSendBufSize);

    m_iStatus = EUSS_OPENED;
    m_iReadBegin = m_iReadEnd = 0;

    return 0;
}

int CUDPSocket::FreeUsrBuffer()
{
    if(m_abyRecvBuffer)
    {
        delete []m_abyRecvBuffer;
        m_abyRecvBuffer = NULL;
    }

    m_iUsrRecvBufSize = 0;
    m_iReadBegin = m_iReadEnd = 0;

    return 0;
}

int CUDPSocket::CloseSocket()
{
    if(m_iSocketFD > 0)
    {
        close(m_iSocketFD);
    }

    m_iSocketFD = -1;
    m_iStatus = EUSS_CLOSED;

    return 0;
}

int CUDPSocket::CreateSocket()
{
    if(m_iStatus != EUSS_CLOSED && m_iSocketFD > 0)
    {
        CloseSocket();
    }

    m_iSocketFD = socket(AF_INET, SOCK_DGRAM, 0);

    if(m_iSocketFD < 0)
    {
        SetErrorNO(EEN_UDP_SOCKET__SOCKET_FAILED);

        m_iStatus = EUSS_CLOSED;
        m_iSocketFD = -1;

        return -1;
    }

    if(m_stUDPSocketCfg.m_szBindIP[0] != '\0')
    {
        int iRet = CSocketUtility::BindSocket(m_iSocketFD, m_stUDPSocketCfg.m_szBindIP, m_stUDPSocketCfg.m_ushBindPort);

        if(iRet)
        {
            CloseSocket();
            SetErrorNO(EEN_UDP_SOCKET__BIND_SOCKET_FAILED);

            return -2;
        }
    }

    return 0;
}

int CUDPSocket::RecvData(char* pszSrcIP /* = NULL */, unsigned short* pushSrcPort /* = NULL */)
{
#ifdef _POSIX_MT_
    pthread_mutex_lock( &m_stMutex );
#endif

    if(m_iSocketFD < 0 || m_iStatus != EUSS_OPENED)
    {
#ifdef _POSIX_MT_
        pthread_mutex_unlock( &m_stMutex );
#endif

        SetErrorNO(EEN_UDP_SOCKET__INVALID_FD);

        return -1;
    }

    if(m_iUsrRecvBufSize <= 0)
    {
#ifdef _POSIX_MT_
        pthread_mutex_unlock( &m_stMutex );
#endif

        SetErrorNO(EEN_UDP_SOCKET__CONFIG_USR_BUF_FAILED);

        return -2;
    }

    if(!m_abyRecvBuffer)
    {
        SetErrorNO(EEN_UDP_SOCKET__NULL_POINTER);

#ifdef _POSIX_MT_
        pthread_mutex_unlock( &m_stMutex );
#endif

        return -1;
    }

    //缓冲区已经清空，复位
    if( m_iReadEnd == m_iReadBegin )
    {
        m_iReadBegin = 0;
        m_iReadEnd = 0;
    }

    if( m_iReadEnd == m_iUsrRecvBufSize )
    {
        if (m_iReadBegin > 0)
        {
            memmove(&m_abyRecvBuffer[0], &m_abyRecvBuffer[m_iReadBegin], m_iReadEnd - m_iReadBegin);
            m_iReadEnd -= m_iReadBegin;
            m_iReadBegin = 0;
        }
        //用户的接收缓冲区已经满
        else
        {
            SetErrorNO(EEN_UDP_SOCKET__USR_RECV_BUF_FULL);

#ifdef _POSIX_MT_
            pthread_mutex_unlock( &m_stMutex );
#endif
            return -2;
        }
    }

    sockaddr_in stTempSockAddr;
    memset((void *)&stTempSockAddr, 0, sizeof( stTempSockAddr ) );
    socklen_t iTempAddrLen = sizeof(stTempSockAddr);

    int iRecvBytes = recvfrom(m_iSocketFD, m_abyRecvBuffer,
                              m_iUsrRecvBufSize, 0, (struct sockaddr *)&stTempSockAddr, (socklen_t *)&iTempAddrLen);

    if(iRecvBytes > 0)
    {
        m_iReadEnd += iRecvBytes;

        if(pszSrcIP != NULL)
        {
            strcpy(pszSrcIP, inet_ntoa(stTempSockAddr.sin_addr));
        }

        if(pushSrcPort != NULL)
        {
            *pushSrcPort = ntohs(stTempSockAddr.sin_port);
        }
    }
    //没有收到数据
    else if(iRecvBytes == 0)
    {
        SetErrorNO(EEN_UDP_SOCKET__NO_DATA_RECIEVED);

#ifdef _POSIX_MT_
        pthread_mutex_unlock( &m_stMutex );
#endif
        return -3;
    }
    //接收发生错误
    if(iRecvBytes < 0)
    {
        if(errno != EAGAIN)
        {
            SetErrorNO(EEN_UDP_SOCKET__RECV_EAGAIN);
        }
        else
        {
            SetErrorNO(EEN_UDP_SOCKET__RECV_FAILED);
        }

#ifdef _POSIX_MT_
        pthread_mutex_unlock( &m_stMutex );
#endif
        return -3;
    }

#ifdef _POSIX_MT_
    pthread_mutex_unlock( &m_stMutex );
#endif

    return 0;
}

int CUDPSocket::FlushRecvBuf(int iFlushLength)
{
    if(m_abyRecvBuffer == NULL || m_iUsrRecvBufSize <= 0)
    {
        SetErrorNO(EEN_UDP_SOCKET__CONFIG_USR_BUF_FAILED);

        return -1;
    }

    if(iFlushLength > m_iReadEnd - m_iReadBegin || iFlushLength == -1)
    {
        iFlushLength = m_iReadEnd - m_iReadBegin;
    }

    m_iReadBegin += iFlushLength;

    if( m_iReadBegin == m_iReadEnd )
    {
        m_iReadBegin = m_iReadEnd = 0;
    }

    return 0;
}

int CUDPSocket::GetData(unsigned char*& rpcDataBeg, int& riDataLenth)
{
    if(m_abyRecvBuffer == NULL || m_iUsrRecvBufSize <= 0)
    {
        riDataLenth = 0;
        SetErrorNO(EEN_UDP_SOCKET__CONFIG_USR_BUF_FAILED);

        return -1;
    }

    rpcDataBeg = &m_abyRecvBuffer[m_iReadBegin];
    riDataLenth = m_iReadEnd - m_iReadBegin;

    return 0;
}

int CUDPSocket::SendDataTo(const unsigned char* pszCodeBuf, int iCodeLength, const char* pszDstIP, unsigned short ushDstPort)
{
    if(m_iSocketFD < 0 || m_iStatus != EUSS_OPENED)
    {
        SetErrorNO(EEN_UDP_SOCKET__INVALID_FD);

        return -1;
    }

    if( !pszCodeBuf || iCodeLength <= 0 || m_iSocketFD < 0 || !pszDstIP ||
            ushDstPort <= 0)
    {
        SetErrorNO(EEN_UDP_SOCKET__NULL_POINTER);

        return -1;
    }

    sockaddr_in stTempSockAddr;
    memset((void *)&stTempSockAddr, 0, sizeof( stTempSockAddr ) );
    stTempSockAddr.sin_family = AF_INET;
    stTempSockAddr.sin_addr.s_addr = inet_addr(pszDstIP);
    stTempSockAddr.sin_port = htons(ushDstPort);

    int iSentBytes = sendto(m_iSocketFD, (const void *)pszCodeBuf,
                            iCodeLength, 0, (const struct sockaddr *)&stTempSockAddr, sizeof(stTempSockAddr));

    if(iSentBytes < 0 || iSentBytes < iCodeLength)
    {
        SetErrorNO(EEN_UDP_SOCKET__SENDTO_FAILED);

        return -3;
    }

    return 0;
}

int CUDPSocket::SendData(const unsigned char* pszCodeBuf, int iCodeLength)
{
    return SendDataTo(pszCodeBuf, iCodeLength,
                      m_stUDPSocketCfg.m_szServerIP, m_stUDPSocketCfg.m_ushServerPort);
}

