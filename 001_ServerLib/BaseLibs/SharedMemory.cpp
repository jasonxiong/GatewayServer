
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//modify by cpp 0712
//#include "LotusLogAdapter.hpp"
//end modify by cpp 0712
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
    //printf("In GenerateShmKey filename:%s,KeyID:%c\n",pszKeyFileName,ucKeyPrjID);
    //TRACESVR("In GenerateShmKey filename:%s,KeyID:%c\n",pszKeyFileName,ucKeyPrjID);
    return m_uiShmKey;
}

int CSharedMemory::AllocateShmSpace(size_t iSize)
{
    m_iShmSize = iSize + sizeof(CSharedMemory);

    //printf("In CharedMemory:size:%d, %d\n",iSize,sizeof(CSharedMemory));

    TRACESVR("Try to alloc shm of %lu bytes, shmkey = %x, shmid = %d\n",
             (unsigned long)m_iShmSize, m_uiShmKey, m_iShmID);
    m_iShmID = shmget(m_uiShmKey, m_iShmSize, IPC_CREAT|IPC_EXCL|0666);

    if(m_iShmID < 0)
    {
        if(errno != EEXIST)
        {
            printf("Alloc shm failed, %s\n", strerror(errno));
            return -2;
        }

        TRACESVR("Same shm (key=%08X) exist, now try to attach it...\n", m_uiShmKey);

        m_iShmID = shmget(m_uiShmKey, m_iShmSize, 0666);
        if(m_iShmID < 0)
        {
            //RACESVR("Attach to shm %d failed, %s. Now try to touch it\n", m_iShmID, strerror(errno));
            m_iShmID = shmget(m_uiShmKey, 0, 0666);
            if(m_iShmID < 0)
            {
                printf("Fatel error, touch to shm failed, %s.\n", strerror(errno));
                return -3;
            }
            else
            {
                //TRACESVR("First remove the exist share memory %d\n", m_iShmID);
                if(shmctl(m_iShmID, IPC_RMID, NULL))
                {
                    printf("Remove shm failed, %s\n", strerror(errno));
                    return -4;
                }
                m_iShmID = shmget(m_uiShmKey, m_iShmSize, IPC_CREAT|IPC_EXCL|0666);
                if(m_iShmID < 0)
                {
                    printf("Fatal error, alloc shm failed, %s\n", strerror(errno));
                    return -5;
                }
            }
        }
        else
        {
            ;//RACESVR("Attach to shm succeed.\n");
        }
    }

    //TRACESVR("Successfully alloced shm, key = %08X, id = %d, size = %d\n", m_uiShmKey, m_iShmID, m_iShmSize);

    m_pszInitialMemoryAddress = (char *)shmat(m_iShmID, NULL, 0);
    if(!m_pszInitialMemoryAddress)
    {
        return -6;
    }
    m_iFreeOffset = sizeof(CSharedMemory);
    //m_pszFreeMemoryAddress = m_pszInitialMemoryAddress;

    WriteShmKey();

    return 0;
}

int CSharedMemory::CreateShmSegment(const char* pszKeyFileName, const unsigned char ucKeyPrjID, size_t iSize)
{
    GenerateShmKey(pszKeyFileName, ucKeyPrjID);

    return AllocateShmSpace(iSize);
}

int CSharedMemory::CreateShmSegmentByKey(key_t uiShmKey, size_t iSize)
{
    m_uiShmKey = uiShmKey;

    return AllocateShmSpace(iSize);
}

int CSharedMemory::UseShmBlock(size_t iSize)
{
    if(iSize <= 0)
    {
        return -1;
    }
    if(iSize > (size_t)GetFreeMemorySize())
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
    if(iSize > (size_t)GetFreeMemorySize())
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



size_t CSharedMemory::GetFreeMemorySize()
{
    return (m_iShmSize - m_iFreeOffset);
}

//写进程分配共享内存的key，
int CSharedMemory::WriteShmKey()
{
    FILE *fFile = NULL;
    fFile = fopen("./Shmid", "wt+");

    if (fFile == NULL)
    {
        return -1;
    }

    fprintf(fFile,"%x\n", m_uiShmKey);

    fclose(fFile);

    return 0;
}
