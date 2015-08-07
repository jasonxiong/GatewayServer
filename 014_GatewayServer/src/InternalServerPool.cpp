#include "InternalServerPool.hpp"
#include "LotusLogAdapter.hpp"


CInternalServerPool::CInternalServerPool()
{

}

void CInternalServerPool::Initialize()
{
    m_uiInternalServerNumber = 0;
    m_pDefaultSocket = 0;
}

TInternalServerSocket* CInternalServerPool::GetSocketByFD(int iFD)
{
    int i = 0;

    //目前这里用遍历，比用hash合算
    for(i = 0; i < (int)m_uiInternalServerNumber; ++i)
    {
        if(iFD == m_astInternalServerSocket[i].m_iSocketFD)
        {
            return &m_astInternalServerSocket[i];
        }
    }

    return NULL;
}

TInternalServerSocket* CInternalServerPool::GetSocketByInternalServerIP(unsigned int uiInternalServerIP,
                                                                        unsigned short ushListenedPort)
{
    int i = 0;

    //目前这里用遍历，比用哈西合算
    for(i = 0; i < (int)m_uiInternalServerNumber; ++i)
    {
        if(uiInternalServerIP == m_astInternalServerSocket[i].m_uiSrcIP &&
            ushListenedPort == m_astInternalServerSocket[i].m_ushListenedPort)
        {
            return &m_astInternalServerSocket[i];
        }
    }

    return NULL;
}

TInternalServerSocket* CInternalServerPool::GetSocketByTypeAndID(unsigned short ushServerType,
                                                                 unsigned short ushServerID)
{
    int i = 0;

    //目前这里用遍历，比用哈西合算
    for(i = 0; i < (int)m_uiInternalServerNumber; ++i)
    {
        if(ushServerType == m_astInternalServerSocket[i].m_ushServerType &&
            ushServerID == m_astInternalServerSocket[i].m_ushServerID)
        {
            return &m_astInternalServerSocket[i];
        }
    }

    return NULL;
}

int CInternalServerPool::AddInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort,
                                             unsigned short ushServerType, unsigned short ushServerID)
{
    if(m_uiInternalServerNumber >= MAX_SERVER_ENTITY_NUMBER)
    {
        return -1;
    }

    int i;
    for(i = 0; i < (int)m_uiInternalServerNumber; ++i)
    {
        if(m_astInternalServerSocket[i].m_uiSrcIP == uiInternalServerIP &&
            m_astInternalServerSocket[i].m_ushListenedPort == ushListenedPort)
        {
            return 0;
        }
    }

    m_astInternalServerSocket[m_uiInternalServerNumber].m_uiSrcIP = uiInternalServerIP;
    m_astInternalServerSocket[m_uiInternalServerNumber].m_ushListenedPort = ushListenedPort;
    m_astInternalServerSocket[m_uiInternalServerNumber].m_iSocketFD = -1;
    m_astInternalServerSocket[m_uiInternalServerNumber].m_ushServerType = ushServerType;
    m_astInternalServerSocket[m_uiInternalServerNumber].m_ushServerID = ushServerID;
    m_uiInternalServerNumber++;

    return 0;
}



//设置默认内部套接字
int CInternalServerPool::SetDefaultSocket(unsigned short ushServerType, unsigned short ushServerID)
{
    int i = 0;

    for(i = 0; i < (int)m_uiInternalServerNumber; ++i)
    {
        if(ushServerType == m_astInternalServerSocket[i].m_ushServerType &&
            ushServerID == m_astInternalServerSocket[i].m_ushServerID)
        {
            m_pDefaultSocket = &m_astInternalServerSocket[i];
            return 0;
        }
    }

    return -1;
}

int CInternalServerPool::ClearDefaultSocket()
{
    m_pDefaultSocket = NULL;

    return 0;
}

//获取默认内部套接字
TInternalServerSocket* CInternalServerPool::GetDefaultSocket()
{
    return m_pDefaultSocket;
}

int CInternalServerPool::ClearInternalServerByIPAndPort( unsigned int uiInternalServerIP, unsigned short ushListenPort )
{
    int i;
    for(i = 0; i < (int)m_uiInternalServerNumber; ++i)
    {
        if(m_astInternalServerSocket[i].m_uiSrcIP == uiInternalServerIP && m_astInternalServerSocket[i].m_ushListenedPort == ushListenPort)
        {
            if(i == (int)m_uiInternalServerNumber - 1)
            {
                m_uiInternalServerNumber--;
            }
            else
            {
                m_astInternalServerSocket[i] = m_astInternalServerSocket[m_uiInternalServerNumber - 1];
                m_uiInternalServerNumber--;
            }

            return 0;
        }
    }

    return 0;
}
