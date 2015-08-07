#ifndef __EXTERNAL_OUTPUT_THREAD_HPP__
#define __EXTERNAL_OUTPUT_THREAD_HPP__

#include <pthread.h>
#include "IOHandler.hpp"

class CExternalOutputThread
{
public:
	int Initialize(CIOHandler* pIOHandler);
	int Run();

private:
	int CreateThread();

private:
	CIOHandler* m_pstIOHandler;

	pthread_t m_hTrd;
	pthread_attr_t m_stAttr;
};


#endif

