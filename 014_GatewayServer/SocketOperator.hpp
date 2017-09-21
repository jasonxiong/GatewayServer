
#ifndef __SOCKET_OPERATOR_HPP__
#define __SOCKET_OPERATOR_HPP__

#include <netinet/in.h>



class CSocketOperator
{
public:
	//接受
	static int Accept(const int iListeningFD, unsigned int& ruiIP, unsigned short& rushPort);
	//关闭
	static int Close(const int iFD);

	//发送
	static int Send(const int iFD, const int iCodeLength, const char* pszCodeBuffer);
	//接收
	static int Recv(const int iFD, const int iCodeLength, const char* pszCodeBuffer);

private:
	//临时对象
	static struct sockaddr_in m_stSocketAddress;
	static int m_iSocketAddressSize;

};

#endif
