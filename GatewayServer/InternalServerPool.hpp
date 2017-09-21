#ifndef __INTERNALSERVER_POOL_HPP__
#define __INTERNALSERVER_POOL_HPP__

#include "ConfigDefine.hpp"
#include "LotusDefine.hpp"

class CInternalServerPool
{
public:
	CInternalServerPool();

	void Initialize();

	TInternalServerSocket* GetSocketByFD(int iFD);
	TInternalServerSocket* GetSocketByInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort);
	TInternalServerSocket* GetSocketByTypeAndID(unsigned short ushServerType, unsigned short ushServerID);

	int AddInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort, 
        unsigned short ushServerType, unsigned short ushServerID);
    int ClearInternalServerByIPAndPort(unsigned int uiInternalServerIP, unsigned short ushListenPort);

	//设置默认内部套接字
	int SetDefaultSocket(unsigned short ushServerType, unsigned short ushServerID);
    int ClearDefaultSocket();

	//获取默认内部套接字
	TInternalServerSocket* GetDefaultSocket();
    
private:

	unsigned int m_uiInternalServerNumber;
	TInternalServerSocket m_astInternalServerSocket[MAX_SERVER_ENTITY_NUMBER];
	TInternalServerSocket* m_pDefaultSocket;

};

#endif
