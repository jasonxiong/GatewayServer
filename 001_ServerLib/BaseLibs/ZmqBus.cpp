
#include "LogAdapter.hpp"
#include "StringUtility.hpp"

#include "ZmqBus.hpp"

using namespace ServerLib;

void* ZmqBus::m_spZmqCtx = NULL;

ZmqBus::ZmqBus()
{
    m_pZmqSocket = NULL;

    memset(m_szSocketAddr, 0, sizeof(m_szSocketAddr));
    memset(m_szRealSockAddr, 0 ,sizeof(m_szRealSockAddr));

    m_iSocketType = -1;
    m_iProcType = -1;
    m_iServiceType = -1;
}

ZmqBus::~ZmqBus()
{
    ZmqFin();
}

int ZmqBus::ZmqInit(const char* pszAddress, enZmqSocketType eSocketType, enZmqProcType eProcType, enZmqServiceType eServiceType)
{
    if(!pszAddress)
    {
        LOGERROR("fail to init zero mq, invalid address!\n");
        return -1;
    }

    m_iSocketType = eSocketType;
    m_iProcType = eProcType;
    m_iServiceType = eServiceType;

    SAFE_STRCPY(m_szSocketAddr, pszAddress, sizeof(m_szSocketAddr)-1);

    if(!m_spZmqCtx)
    {
        m_spZmqCtx = zmq_init(1);
        if(!m_spZmqCtx)
        {
            LOGERROR("fail to zmq_init, contex is NULL!\n");
            return -2;
        }
    }

    m_pZmqSocket = zmq_socket(m_spZmqCtx, GetSocketType(eSocketType));
    if(!m_pZmqSocket)
    {
        LOGERROR("fail to zmq_socket, socket type %d\n", eSocketType);
        return -3;
    }

    //先根据进程模型生成实际地址
    int iRet = GetRealSockAddr(eProcType, m_szSocketAddr);
    if(iRet)
    {
        LOGERROR("fail to generate zmq addr, proc type %u, socket address %s\n", eProcType, m_szSocketAddr);
        return -4;
    }

    //根据zmq提供服务类型来简历连接
    switch(eServiceType)
    {
    case EN_ZMQ_SERVICE_SERVER:
    {
        iRet = zmq_bind(m_pZmqSocket, m_szRealSockAddr);
    }
    break;

    case EN_ZMQ_SERVICE_CLIENT:
    {
        iRet = zmq_connect(m_pZmqSocket, m_szRealSockAddr);
    }
    break;

    default:
    {
        LOGERROR("fail to establish zmq socket, invalid service type %u\n", eServiceType);
        return -5;
    }
    break;
    }

    if(iRet)
    {
        LOGERROR("fail to establish zmq socket, ret %d\n", iRet);
        return -6;
    }

    return 0;
}

int ZmqBus::ZmqSend(const char* pszBuff, int iMsgLen, int iFlags)
{
    if(!pszBuff)
    {
        return -1;
    }

    int iRet = zmq_send(m_pZmqSocket, pszBuff, iMsgLen, iFlags);
    if(iRet < 0)
    {
        LOGERROR("fail to zmq_send, zmq_send ret %d\n", iRet);
        return -2;
    }

    return 0;
}

int ZmqBus::ZmqRecv(char* pszBuff, int iMaxLen, int& iMsgLen, int iFlags)
{
    if(!pszBuff)
    {
        return -1;
    }

    int iRecvLen = zmq_recv(m_pZmqSocket, pszBuff, iMaxLen, iFlags | ZMQ_DONTWAIT);
    if(iRecvLen < 0)
    {
        if(errno == EAGAIN)
        {
            return 0;
        }

        LOGERROR("fail to zmq_recv, zmq_recv ret %d\n", iRecvLen);
        return -2;
    }

    iMsgLen = iRecvLen;

    return 0;
}

void ZmqBus::ZmqFin()
{
    if(m_pZmqSocket)
    {
        zmq_close(m_pZmqSocket);

        m_pZmqSocket = NULL;
    }

    return ;
}

int ZmqBus::GetSocketType(enZmqSocketType eSocketType)
{
    switch(eSocketType)
    {
    case EN_ZMQ_SOCKET_PAIR:
    {
        return ZMQ_PAIR;
    }
    break;

    case EN_ZMQ_SOCKET_PUB:
    {
        return ZMQ_PUB;
    }
    break;

    case EN_ZMQ_SOCKET_SUB:
    {
        return ZMQ_SUB;
    }
    break;

    case EN_ZMQ_SOCKET_REQ:
    {
        return ZMQ_REQ;
    }
    break;

    case EN_ZMQ_SOCKET_REP:
    {
        return ZMQ_REP;
    }
    break;

    default:
    {
        return -1;
    }
    break;
    }

    return -1;
}

int ZmqBus::GetRealSockAddr(enZmqProcType eProcType, const char* pszAddress)
{
    if(!pszAddress)
    {
        LOGERROR("fail to get real zmq socket addr, invalid param!\n");
        return -1;
    }

    switch(eProcType)
    {
    case EN_ZMQ_PROC_INPROC:
    {
        SAFE_SPRINTF(m_szRealSockAddr, sizeof(m_szRealSockAddr)-1, "inproc://%s", m_szSocketAddr);
    }
    break;

    case EN_ZMQ_PROC_IPC:
    {
        SAFE_SPRINTF(m_szRealSockAddr, sizeof(m_szRealSockAddr)-1, "ipc:///%s", m_szSocketAddr);
    }
    break;

    case EN_ZMQ_PROC_TCP:
    {
        SAFE_SPRINTF(m_szRealSockAddr, sizeof(m_szRealSockAddr)-1, "tcp://%s", m_szSocketAddr);
    }
    break;

    default:
    {

    }
    break;
    }

    return 0;
}
