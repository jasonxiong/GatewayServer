
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "LotusLogAdapter.hpp"
#include "SharedMemory.hpp"

key_t CSharedMemory::GenerateShmKey(const char* pszKeyFileName, const unsigned char ucKeyPrjID)
{
	if(!pszKeyFileName)
	{
		return 0;
	}

    char szCmd[1024] = {0};
    sprintf(szCmd, "touch %s", pszKeyFileName);
    system(szCmd);

	m_uiShmKey = ftok(pszKeyFileName, ucKeyPrjID);
	return m_uiShmKey;
}

int CSharedMemory::AllocateShmSpace(size_t iSize)
{
	m_iShmSize = iSize + sizeof(CSharedMemory);

	TRACESVR(LOG_LEVEL_DETAIL, "Try to alloc shm of %d bytes...\n", m_iShmSize);

	m_iShmID = shmget(m_uiShmKey, m_iShmSize, IPC_CREAT|IPC_EXCL|0666);

	if(m_iShmID < 0)
	{
		if(errno != EEXIST)
		{
			TRACESVR(LOG_LEVEL_DETAIL, "Alloc shm failed, %s\n", strerror(errno));
			return -2;
		}

		TRACESVR(LOG_LEVEL_DETAIL, "Same shm (key=%08X) exist, now try to attach it...\n", m_uiShmKey);

		m_iShmID = shmget(m_uiShmKey, m_iShmSize, 0666);
		if(m_iShmID < 0)
		{
			TRACESVR(LOG_LEVEL_DETAIL, "Attach to shm %d failed, %s. Now try to touch it\n", m_iShmID, strerror(errno));
			m_iShmID = shmget(m_uiShmKey, 0, 0666);
			if(m_iShmID < 0)
			{
				TRACESVR(LOG_LEVEL_DETAIL, "Fatel error, touch to shm failed, %s.\n", strerror(errno));
				return -3;
			}
			else
			{
				TRACESVR(LOG_LEVEL_DETAIL, "First remove the exist share memory %d\n", m_iShmID);
				if(shmctl(m_iShmID, IPC_RMID, NULL))
				{
					TRACESVR(LOG_LEVEL_DETAIL, "Remove shm failed, %s\n", strerror(errno));
					return -4;
				}
				m_iShmID = shmget(m_uiShmKey, m_iShmSize, IPC_CREAT|IPC_EXCL|0666);
				if(m_iShmID < 0)
				{
					TRACESVR(LOG_LEVEL_DETAIL, "Fatal error, alloc shm failed, %s\n", strerror(errno));
					return -5;
				}
			}
		}
		else
		{
			TRACESVR(LOG_LEVEL_DETAIL, "Attach to shm succeed.\n");
		}
	}

	TRACESVR(LOG_LEVEL_DETAIL, "Successfully alloced shm, key = %08X, id = %d, size = %d\n", m_uiShmKey, m_iShmID, m_iShmSize);

	m_pszInitialMemoryAddress = (char *)shmat(m_iShmID, NULL, 0);
	if(!m_pszInitialMemoryAddress)
	{
		return -6;
	}
	m_iFreeOffset = sizeof(CSharedMemory);

	return 0;
}

int CSharedMemory::CreateShmSegment(const char* pszKeyFileName, const unsigned char ucKeyPrjID, size_t iSize)
{
	GenerateShmKey(pszKeyFileName, ucKeyPrjID);

	return AllocateShmSpace(iSize);
}

int CSharedMemory::UseShmBlock(size_t iSize)
{
	if(iSize <= 0)
	{
		return -1;
	}
	if(iSize > (size_t)GetFreeMemoryLength())
	{
		return -2;
	}

	m_iFreeOffset += iSize;

	return 0;
}

int CSharedMemory::UseShmBlock(size_t iSize, char* & rpszShm)
{
	if(iSize <= 0)
	{
		return -1;
	}
	if(iSize > (size_t)GetFreeMemoryLength())
	{
		return -2;
	}

	rpszShm = m_pszInitialMemoryAddress + m_iFreeOffset;
	m_iFreeOffset += iSize;

	return 0;
}

const char* CSharedMemory::GetFreeMemoryAddress()
{
	return m_pszInitialMemoryAddress + m_iFreeOffset;
}

const char* CSharedMemory::GetInitialMemoryAddress()
{
	return m_pszInitialMemoryAddress;
}



//////////////////////////////////////////////////////////////////////////



int CSharedMemory::GetFreeMemoryLength()
{
	return (m_iShmSize - m_iFreeOffset);
}


