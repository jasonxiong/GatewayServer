#ifndef __FDPOOL_HPP__
#define __FDPOOL_HPP__


#include "LotusDefine.hpp"
#include "ConfigDefine.hpp"
#include "InternalServerPool.hpp"
#include "ExternalClientPool.hpp"

class CFDPool
{
public:

	//初始化External、Internal套接字池
	int Initialize();


	//将指定FD设置为空
	void SetFDInactive(int iFD);

	// 设置激活
	void SetFDActive(int iFD);

	//是否是外部监听套接字
	bool IsListeningForExternalClient(int iFD);
	//设置为外部监听套接字
	void SetListeningForExternalClient(int iFD);
	//是否是内部监听套接字
	bool IsListeningForInternalServer(int iFD);
	//设置为内部监听套接字
	void SetListeningForInternalServer(int iFD);


	//是否是外部连接套接字
	bool IsConnectedByExternalClient(int iFD);
	//设置为外部连接套接字
	void SetConnectedByExternalClient(int iFD);
	//是否是内部连接套接字
	bool IsConnectedByInternalServer(int iFD);
	//设置为内部连接套接字
	void SetConnectedByInternalServer(int iFD);


	//增加内部Server地址
    int AddInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort, 
        unsigned short ushServerType, unsigned short ushServerID);
    int ClearInternalServerByIPAndPort(unsigned int uiInternalServerIP, unsigned short ushListenedPort);

	TInternalServerSocket* GetInternalSocketByTypeAndID(unsigned short ushServerType, unsigned short ushServerID);
	TInternalServerSocket* GetInternalSocketByInternalServerIP(unsigned int uiInternalServerIP, unsigned short ushListenedPort);
	TInternalServerSocket* GetInternalSocketByFD(int iFD);
	TExternalClientSocket* GetExternalSocketByFD(int iFD);
	TExternalClientSocket* GetExternalFirstSocket();


    //设置默认内部套接字
    int SetDefaultInternalSocket(unsigned short ushServerType, unsigned short ushServerID);
    //获取默认内部套接字
    TInternalServerSocket* GetDefaultInternalSocket();
    //清除默认套接字
    int ClearDefaultInternalSocket();
    
private:
	unsigned int m_auiSocketFlag[FD_SIZE];
	CInternalServerPool m_stInternalServerPool;
	CExternalClientPool m_stExternalClientPool;

};

#endif
