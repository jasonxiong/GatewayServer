#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "LotusDefine.hpp"
#include "LotusLogAdapter.hpp"
#include "SocketOperator.hpp"


struct sockaddr_in CSocketOperator::m_stSocketAddress;
int CSocketOperator::m_iSocketAddressSize = sizeof(m_stSocketAddress);

int CSocketOperator::Accept(const int iListeningFD, unsigned int& ruiIP, unsigned short& rushPort)
{
	int iFD = -1;
	iFD = accept(iListeningFD, (struct sockaddr *)&m_stSocketAddress, (socklen_t *)&m_iSocketAddressSize);
	if(iFD < 0)
	{
		return -1;
	}
	if(iFD >= (int)FD_SIZE)
	{
		Close(iFD);
		return -2;
	}

	ruiIP = m_stSocketAddress.sin_addr.s_addr;
	rushPort = m_stSocketAddress.sin_port;

	return iFD;
}

int CSocketOperator::Close(int iFD)
{
    int iRet = close(iFD);
	return iRet;
}

int CSocketOperator::Send(const int iFD, const int iCodeLength, const char* pszCodeBuffer)
{
	unsigned int uiSendBeginOffset = 0;
	int iLeftLength = iCodeLength;

	int iSendBytes = 0;
	int iErrorNo = 0;
	bool bClose = false;
	while(iLeftLength > 0)
	{
		iSendBytes = send(iFD, pszCodeBuffer + uiSendBeginOffset, iLeftLength, 0);
		iErrorNo = errno;;
		if(iSendBytes <= 0)
		{
			//重新发报文
			if(iErrorNo == EINTR)
			{
				continue;
			}
			//缓冲不足以发报文
			else if(iErrorNo == EAGAIN)
			{
				bClose = false;
				break;
			}

			//出错
			bClose = true;
			break;
		}

		iLeftLength -= iSendBytes;
		uiSendBeginOffset += iSendBytes;
	}

    //没发完,重置连接
	if(iLeftLength > 0)
	{
		//TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::Send, FD:%d CodeLength:%d LeftLength:%d ErrorNo:%d\n", iFD, iCodeLength, iLeftLength, iErrorNo);
	}

	if(bClose)
	{
		//TRACESVR(LOG_LEVEL_DETAIL, "In CIOHandler::Send, Close:1 FD:%d CodeLength:%d SendBytes:%d ErrorNo:%d\n", iFD, iCodeLength, iSendBytes, iErrorNo);
		return -2;
	}

	return (int)uiSendBeginOffset;
}

//-1 错误 -2 关闭 0 缓冲区满 >0 接收字节数
int CSocketOperator::Recv(const int iFD, const int iCodeLength, const char* pszCodeBuffer)
{
	if (iFD < 0 || !pszCodeBuffer)
	{
		return -1;
	}

	//传入参数为0时，认为缓存不够
	if (!iCodeLength )
	{
		TRACESVR(LOG_LEVEL_DETAIL, "In CSocketOperator::Recv, No Recv Buffer\n");
		return 0;
	}

	unsigned int uiRecvEndOffset = 0;
	int iLeftLength = iCodeLength;

	int iRecvBytes = 0;
	int iErrorNo = 0;
	bool bClose = false;

	do{
        //收满了
		if(iLeftLength <= 0)
		{
			bClose = false;
			break;
		}

		iRecvBytes = recv(iFD, (char*)pszCodeBuffer + uiRecvEndOffset, iLeftLength, 0);

		if (iRecvBytes > 0)
		{		
			//偏移，继续收报文
			uiRecvEndOffset += iRecvBytes;
			iLeftLength -= iRecvBytes;
		}
		else if(iRecvBytes < 0)
		{
			iErrorNo = errno;

			if (iErrorNo != EAGAIN)
			{
                //错误发生
				bClose = true;
                TRACESVR(LOG_LEVEL_DETAIL, "In CSocketOperator::Recv, Error in Read:%d!\n", iErrorNo);
				break;
			}
		}
		else if(iRecvBytes == 0)
		{
            //关闭连接
			bClose = true;
			break;
		}

	}while(iRecvBytes > 0);

	if(bClose)
	{
        if(iRecvBytes == 0)
        {
            return -2;
        }
        else
        {
            return -1;
        }
	}

	return uiRecvEndOffset;
}
