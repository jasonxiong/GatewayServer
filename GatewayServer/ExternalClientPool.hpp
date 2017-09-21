#ifndef __EXTERNALCLIENT_POOL_HPP__
#define __EXTERNALCLIENT_POOL_HPP__

#include "LotusDefine.hpp"

class CExternalClientPool
{
public:
	void Initialize();

	TExternalClientSocket* GetSocketByFD(const int iFD);
	TExternalClientSocket* GetFirstSocket();

	void DeleteSocketByFD(const int iFD);
	void AddSocketByFD(const int iFD);

private:
	TExternalClientSocket m_astExternalSocket[FD_SIZE];
	
	TExternalClientSocket* m_pFirstSocket;
};

#endif
