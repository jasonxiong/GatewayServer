#include <assert.h>
#include "FDPool.hpp"
#include "base.hpp"
#include "LotusLogAdapter.hpp"


int CFDPool::Initialize()
{
	memset(m_auiSocketFlag, 0, sizeof(m_auiSocketFlag));
	m_stInternalServerPool.Initialize();
	m_stExternalClientPool.Initialize();

	return 0;
}

void CFDPool::SetFDInactive(int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

    // 为以防万一, internal和external的都关闭
    m_stExternalClientPool.DeleteSocketByFD(iFD);
    TInternalServerSocket *pInSocket = m_stInternalServerPool.GetSocketByFD(iFD);
    if (pInSocket)
    {
        pInSocket->m_iSocketFD = -1;
    }

	m_auiSocketFlag[iFD] = 0;

	return;
}

void CFDPool::SetFDActive(int iFD)
{
	assert(iFD > 0 && iFD < (int)FD_SIZE);

	m_stExternalClientPool.AddSocketByFD(iFD);

}

bool CFDPool::IsListeningForExternalClient(int iFD)
{
	return (m_auiSocketFlag[iFD] & ELSF_ListeningForExternalClient);
}

void CFDPool::SetListeningForExternalClient(int iFD)
{
	m_auiSocketFlag[iFD] = ELSF_ListeningForExternalClient;
}

bool CFDPool::IsListeningForInternalServer(int iFD)
{
	return (m_auiSocketFlag[iFD] & ELSF_ListeningForInternalServer);
}

void CFDPool::SetListeningForInternalServer(int iFD)
{
	m_auiSocketFlag[iFD] = ELSF_ListeningForInternalServer;
}


bool CFDPool::IsConnectedByExternalClient(int iFD)
{
	return (m_auiSocketFlag[iFD] & ELSF_ConnectedByExternalClient);
}

void CFDPool::SetConnectedByExternalClient(int iFD)
{
	m_auiSocketFlag[iFD] = ELSF_ConnectedByExternalClient;
}

bool CFDPool::IsConnectedByInternalServer(int iFD)
{
	return (m_auiSocketFlag[iFD] & ELSF_ConnectedByInternalServer);
}

void CFDPool::SetConnectedByInternalServer(int iFD)
{
	m_auiSocketFlag[iFD] = ELSF_ConnectedByInternalServer;
}

int CFDPool::AddInternalServerIP( unsigned int uiInternalServerIP, unsigned short ushListenedPort, 
                                 unsigned short ushServerType, unsigned short ushServerID)
{
    return m_stInternalServerPool.AddInternalServerIP(uiInternalServerIP, ushListenedPort, ushServerType, ushServerID);
}

int CFDPool::ClearInternalServerByIPAndPort( unsigned int uiInternalServerIP, unsigned short ushListenedPort )
{
    return m_stInternalServerPool.ClearInternalServerByIPAndPort(uiInternalServerIP, ushListenedPort);
}

TInternalServerSocket* CFDPool::GetInternalSocketByTypeAndID( unsigned short ushServerType, unsigned short ushServerID )
{
    return m_stInternalServerPool.GetSocketByTypeAndID(ushServerType, ushServerID);
}

TInternalServerSocket* CFDPool::GetInternalSocketByInternalServerIP( unsigned int uiInternalServerIP, unsigned short ushListenedPort )
{
    return m_stInternalServerPool.GetSocketByInternalServerIP(uiInternalServerIP, ushListenedPort);
}

TInternalServerSocket* CFDPool::GetInternalSocketByFD( int iFD )
{
    return m_stInternalServerPool.GetSocketByFD(iFD);
}

TExternalClientSocket* CFDPool::GetExternalSocketByFD( int iFD )
{
    return m_stExternalClientPool.GetSocketByFD(iFD);
}

TExternalClientSocket* CFDPool::GetExternalFirstSocket()
{
	return m_stExternalClientPool.GetFirstSocket();
}

int CFDPool::SetDefaultInternalSocket( const unsigned short ushServerType, const unsigned short ushServerID )
{
    return m_stInternalServerPool.SetDefaultSocket(ushServerType, ushServerID);
}

int CFDPool::ClearDefaultInternalSocket()
{
    return m_stInternalServerPool.ClearDefaultSocket();
}

TInternalServerSocket* CFDPool::GetDefaultInternalSocket()
{
    return m_stInternalServerPool.GetDefaultSocket();
}
