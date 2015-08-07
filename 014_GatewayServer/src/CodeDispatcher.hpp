#ifndef __CODE_DISPATCHER_HPP__
#define __CODE_DISPATCHER_HPP__


#include "LotusDefine.hpp"
#include "CodeQueue.hpp"
#include "MsgStatistic.hpp"

class CIOHandler;
class CCodeQueueAssemble;
class CFDPool;

class CCodeDispatcher
{
public:
	int Initialize();

	//派发来自外部的消息
	int DispatchOneCode(const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
		unsigned short ushCodeLength, int iSrcFD);

	//向默认目标派发
	int DispatchToDefault(const char* pszDispatchingBuffer, unsigned short ushCodeOffset,
		unsigned short ushCodeLength, int iSrcFD);
	
	//向指定CodeQueue派发
	// @param uiRealUin: 实际派发Uin 不能使用Fd中Uin 
	//                   因为回包时 Fd中的Uin本来就跟uiRealUin不一定一致
	int DispatchToCodeQueue(int iCodeQueueID, const char* pszDispatchingBuffer,
		unsigned short ushCodeOffset, unsigned short ushCodeLength, int iSrcFD, 
		unsigned int uiRealUin);
	
	//向指定内部服务派发
	int DispatchToDstServer(const unsigned short ushServerType, unsigned short ushServerID,
		const char* pszDispatchingBuffer, unsigned short ushCodeOffset, unsigned short ushCodeLength, int iSrcFD, 
		unsigned int uiRealUin);

	//包装，使用NetHead_V2
	int PackNetHead_V2(char* pNetHead, int iSrcFD, unsigned int uiRealUin);

	int SendToInternalServer(TInternalServerSocket* pstSocket,
		int iCodeLength, const char* pszCodeBuffer);

	int SendRemain(TInternalServerSocket* pstSocket);

public:

    CIOHandler* m_pIOHandler;
    CConfigAssemble* m_pstConfigAssemble;
    CCodeQueueAssemble* m_pstCodeQueueAssemble;
    CFDPool* m_pstFDPool;
};

#endif
