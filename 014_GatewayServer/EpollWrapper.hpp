
#ifndef __EPOLL_WRAPPER_HPP__
#define __EPOLL_WRAPPER_HPP__

#include <sys/epoll.h>
#include <assert.h>

#include "LotusDefine.hpp"

#ifdef WIN32

enum EPOLL_EVENTS
{
	EPOLLIN = 0x001,
#define EPOLLIN EPOLLIN
	EPOLLPRI = 0x002,
#define EPOLLPRI EPOLLPRI
	EPOLLOUT = 0x004,
#define EPOLLOUT EPOLLOUT
	EPOLLRDNORM = 0x040,
#define EPOLLRDNORM EPOLLRDNORM
	EPOLLRDBAND = 0x080,
#define EPOLLRDBAND EPOLLRDBAND
	EPOLLWRNORM = 0x100,
#define EPOLLWRNORM EPOLLWRNORM
	EPOLLWRBAND = 0x200,
#define EPOLLWRBAND EPOLLWRBAND
	EPOLLMSG = 0x400,
#define EPOLLMSG EPOLLMSG
	EPOLLERR = 0x008,
#define EPOLLERR EPOLLERR
	EPOLLHUP = 0x010,
#define EPOLLHUP EPOLLHUP
	EPOLLONESHOT = (1 << 30),
#define EPOLLONESHOT EPOLLONESHOT
	EPOLLET = (1 << 31)
#define EPOLLET EPOLLET
};



/* Valid opcodes ( "op" parameter ) to issue to epoll_ctl().  */
#define EPOLL_CTL_ADD 1 /* Add a file decriptor to the interface.  */
#define EPOLL_CTL_DEL 2 /* Remove a file decriptor from the interface.  */
#define EPOLL_CTL_MOD 3 /* Change file decriptor epoll_event structure.  */



typedef union epoll_data
{
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

struct epoll_event
{
	uint32_t events;      /* Epoll events */
	epoll_data_t data;    /* User data variable */
};

/* Creates an epoll instance.  Returns an fd for the new instance.
The "size" parameter is a hint specifying the number of file
descriptors to be associated with the new instance.  The fd
returned by epoll_create() should be closed with close().  */
extern int epoll_create (int __size) __THROW;

/* Manipulate an epoll instance "epfd". Returns 0 in case of success,
-1 in case of error ( the "errno" variable will contain the
specific error code ) The "op" parameter is one of the EPOLL_CTL_*
constants defined above. The "fd" parameter is the target of the
operation. The "event" parameter describes which events the caller
is interested in and any associated user data.  */
extern int epoll_ctl (int __epfd, int __op, int __fd,
struct epoll_event *__event) __THROW;

/* Wait for events on an epoll instance "epfd". Returns the number of
triggered events returned in "events" buffer. Or -1 in case of
error with the "errno" variable set to the specific error code. The
"events" parameter is a buffer that will contain triggered
events. The "maxevents" is the maximum number of events to be
returned ( usually size of "events" ). The "timeout" parameter
specifies the maximum wait time in milliseconds (-1 == infinite).  */
extern int epoll_wait (int __epfd, struct epoll_event *__events, int __maxevents, int __timeout) __THROW;

#endif

typedef int (*Function_EpollHandler) (int);

class CEpollWrapper
{
public:

	void SetHandler_Error(Function_EpollHandler pfError);
	void SetHandler_Read(Function_EpollHandler pfRead);
	void SetHandler_Write(Function_EpollHandler pfWrite);

public:

	int EpollCreate(int iFDSize);
	int EpollWait();

	int EpollAdd(int iFD);
	int EpollDelete(int iFD);

	int SetNonBlock(int iFD);
    int SetNagleOff(int iFD);

	inline void SetEpollWaitingTime(int iEpollWaitingTime);

private:
	bool IsErrorEvent(unsigned int uiEvent, int iFD);
	bool IsReadEvent(unsigned int uiEvent);
	bool IsWriteEvent(unsigned int uiEvent);

	int NotifyErrorEvent(int iFD);
	int NotifyReadEvent(int iFD);
	int NotifyWriteEvent(int iFD);

private:

	Function_EpollHandler m_pfError;
	Function_EpollHandler m_pfRead;
	Function_EpollHandler m_pfWrite;

private:

	int m_iEpollFD;
	int m_iEpollEventSize;
	int m_iEpollWaitingTime;
	epoll_event m_astEpollEvent[FD_SIZE];
	epoll_event m_stOneEpollEvent;

};

inline void CEpollWrapper::SetEpollWaitingTime(int iEpollWaitingTime)
{
	m_iEpollWaitingTime = iEpollWaitingTime;
}



#endif
