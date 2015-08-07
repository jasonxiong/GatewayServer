
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "EpollWrapper.hpp"
#include <errno.h>
#include "LotusLogAdapter.hpp"

void CEpollWrapper::SetHandler_Error(Function_EpollHandler pfError)
{
	m_pfError = pfError;
}

void CEpollWrapper::SetHandler_Read(Function_EpollHandler pfRead)
{
	m_pfRead = pfRead;
}

void CEpollWrapper::SetHandler_Write(Function_EpollHandler pfWrite)
{
	m_pfWrite = pfWrite;
}



int CEpollWrapper::EpollCreate(int iFDSize)
{
	m_iEpollEventSize = iFDSize;
	m_iEpollFD = epoll_create(m_iEpollEventSize);
	if(m_iEpollFD < 0)
	{
		return -1;
	}

	memset(&m_stOneEpollEvent, 0, sizeof(m_stOneEpollEvent));
	//m_stOneEpollEvent.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
	m_stOneEpollEvent.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	m_stOneEpollEvent.data.ptr = NULL;
	m_stOneEpollEvent.data.fd  = -1;

	return 0;
}

int CEpollWrapper::EpollWait()
{
	int iEpollEventNumber;
	iEpollEventNumber = epoll_wait(m_iEpollFD, &m_astEpollEvent[0], m_iEpollEventSize, m_iEpollWaitingTime);
	if(iEpollEventNumber < 0)
	{
		if (errno != EINTR)
		{
			TRACESVR(LOG_LEVEL_ALERT, "epoll_wait Err!Errno=%d,ErrStr=%s\n", errno, strerror(errno));
		}
		
		return -1;
	}
	int i;
	int iFD;
	unsigned int uiEpollEvent;
	for(i = 0; i < iEpollEventNumber; ++i)
	{
		iFD = m_astEpollEvent[i].data.fd;
		uiEpollEvent = m_astEpollEvent[i].events;
		
		if(IsReadEvent(uiEpollEvent))
		{
			NotifyReadEvent(iFD);
		}
		else if(IsWriteEvent(uiEpollEvent))
		{
			NotifyWriteEvent(iFD);
		}
		else if(IsErrorEvent(uiEpollEvent, iFD))
		{
			NotifyErrorEvent(iFD);
		}
	}

	return 0;
}

int CEpollWrapper::EpollAdd(int iFD)
{
	int iRet = 0;
	m_stOneEpollEvent.data.fd = iFD;
	iRet = epoll_ctl(m_iEpollFD, EPOLL_CTL_ADD, iFD, &m_stOneEpollEvent);
	if(iRet < 0)
	{
		return -1;
	}
	return 0;
}

int CEpollWrapper::EpollDelete(int iFD)
{
	int iRet = 0;
	iRet = epoll_ctl(m_iEpollFD, EPOLL_CTL_DEL, iFD, &m_stOneEpollEvent);
	if(iRet < 0)
	{
		return -1;
	}
	return 0;
}

bool CEpollWrapper::IsErrorEvent(unsigned int uiEvent, int iFD)
{
	if((EPOLLERR | EPOLLHUP) & uiEvent)
	{
		TRACESVR(LOG_LEVEL_ALERT, "ErrorEvent: FD = %d, event = %u, error = %d, hup = %d, errno = %d\n", 
			iFD, uiEvent, uiEvent & EPOLLERR, uiEvent & EPOLLHUP, errno);
		return true;
	}
	return false;
}

bool CEpollWrapper::IsReadEvent(unsigned int uiEvent)
{
	if((EPOLLIN) & uiEvent)
	{
		return true;
	}
	return false;
}

bool CEpollWrapper::IsWriteEvent(unsigned int uiEvent)
{
	if((EPOLLOUT) & uiEvent)
	{
		return true;
	}
	return false;
}

int CEpollWrapper::NotifyErrorEvent(int iFD)
{
	(*m_pfError)(iFD);
	return 0;
}

int CEpollWrapper::NotifyReadEvent(int iFD)
{
	(*m_pfRead)(iFD);
	return 0;
}

int CEpollWrapper::NotifyWriteEvent(int iFD)
{
	(*m_pfWrite)(iFD);
	return 0;
}

//////////////////////////////////////////////////////////////////////////

int CEpollWrapper::SetNonBlock(int iFD)
{
#ifdef WIN32
	unsigned int uiNonBlock = 1;
	ioctlsocket(iFD, FIONBIO, &uiNonBlock);
#else
	int iFlags;
	iFlags = fcntl(iFD, F_GETFL, 0);
	iFlags |= O_NONBLOCK;
	//iFlags |= O_NDELAY;
	fcntl(iFD, F_SETFL, iFlags);
#endif
	return 0;
}

int CEpollWrapper::SetNagleOff(int iFD)
{
    /* Disable the Nagle (TCP No Delay) algorithm */ 
    int flag = 1; 

    setsockopt(iFD, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    return 0;
}
