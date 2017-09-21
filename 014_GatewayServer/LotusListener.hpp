#ifndef __LOTUS_LISTENER_HPP__
#define __LOTUS_LISTENER_HPP__

enum 
{
	LotusListener_UnListen = 1,
	LotusListener_Listen = 2,
};

class CLotusListener
{
public:
	CLotusListener();
	~CLotusListener();
	
	int Init(unsigned int uiIP, unsigned short ushPort,
		unsigned int uiSendBufferSize, bool bExternalClient);

	inline int GetFD();

	int CreateTCPSocket();
	int Bind(unsigned int uiIP, unsigned short ushPort);
	int Listen();

	int SetSendBufferSize(int iBufferSize);

	int SetForEpoll();

private:

	int SetReuseAddress();
	int SetKeepAlive();
	int SetLingerOff();

public:
	unsigned int m_uiIP;
	unsigned short m_ushPort;
	unsigned int m_uiSendBufferSize;
	bool m_bExternalClient;
	int m_iState; 

private:
	int m_iListeningSocket;
};

int CLotusListener::GetFD()
{
	return m_iListeningSocket;
}

#endif

