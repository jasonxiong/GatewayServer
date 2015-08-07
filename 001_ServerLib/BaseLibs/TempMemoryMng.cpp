/**
*@file TempMemoryMng.cpp
*@author jasonxiong
*@date 2009-11-28
*@version 1.0
*@brief 临时内存管理器的实现文件
*
*
*/

#include <assert.h>

#include "TempMemoryMng.hpp"
#include "ErrorDef.hpp"
#include "SharedMemory.hpp"
#include "LogAdapter.hpp"

using namespace ServerLib;

CTempMemoryMng::CTempMemoryMng()
{
}

CTempMemoryMng::CTempMemoryMng(int iObjSize, int iMaxObjCount)
{
    m_aucObjMem = NULL;
    m_astTmpMemObj = NULL;

    int iMemorySize = iMaxObjCount * iObjSize;
    __ASSERT_AND_LOG(iMemorySize > 0);

    m_iMaxObjCount = iMaxObjCount;
    m_iObjSize = iObjSize;
    m_iMemAllocType = ETMT_ALLOC_BY_MEMORY_MNG;

    m_aucObjMem = new unsigned char[m_iObjSize * m_iMaxObjCount];
    m_astTmpMemObj = new TTempMemoryObj[m_iMaxObjCount];

    __ASSERT_AND_LOG(m_aucObjMem && m_astTmpMemObj);

    Initialize();
}

//void* CTempMemoryMng::operator new(unsigned int uiSize, const void* pThis) throw()
//{
//	if(!pThis)
//	{
//		return NULL;
//	}
//
//	return (void*)pThis;
//}

CTempMemoryMng::~CTempMemoryMng()
{
    if(m_iMemAllocType == ETMT_ALLOC_BY_MEMORY_MNG)
    {
        if(m_astTmpMemObj)
        {
            delete []m_astTmpMemObj;
            m_astTmpMemObj = NULL;
        }

        if(m_aucObjMem)
        {
            delete []m_aucObjMem;
            m_aucObjMem = NULL;
        }
    }
}

CTempMemoryMng* CTempMemoryMng::CreateBySharedMemory(const char* pszKeyFileName,
        const unsigned char ucKeyPrjID,
        int iObjSize, int iMaxObjCount)
{
    if(pszKeyFileName == NULL || iObjSize <= 0 || iMaxObjCount <= 0)
    {
        return NULL;
    }

    CSharedMemory stSharedMemory;
    unsigned int uiSharedMemorySize = sizeof(CTempMemoryMng) + iObjSize * iMaxObjCount +
                                      iMaxObjCount * sizeof(TTempMemoryObj);

    int iRet = stSharedMemory.CreateShmSegment(pszKeyFileName, ucKeyPrjID,
               (int)uiSharedMemorySize);

    if(iRet)
    {
        return NULL;
    }

    //在共享内存的地址上分配CTempMemoryMng
    CTempMemoryMng* pstTempMemMng = (CTempMemoryMng*)stSharedMemory.GetFreeMemoryAddress();

    if(!pstTempMemMng)
    {
        return NULL;
    }

    pstTempMemMng->m_iMaxObjCount = iMaxObjCount;
    pstTempMemMng->m_iObjSize = iObjSize;
    pstTempMemMng->m_iMemAllocType = ETMT_ALLOC_BY_SHARED_MEMEORY;
    pstTempMemMng->m_aucObjMem = (unsigned char*)stSharedMemory.GetFreeMemoryAddress() + sizeof(CTempMemoryMng);
    pstTempMemMng->m_astTmpMemObj = (TTempMemoryObj*)((unsigned char*)stSharedMemory.GetFreeMemoryAddress() +
                                    sizeof(CTempMemoryMng) + iObjSize * iMaxObjCount);

    return pstTempMemMng;
}

int CTempMemoryMng::Initialize()
{
    if(m_astTmpMemObj == NULL || m_aucObjMem == NULL)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__NULL_POINTER);

        return -1;
    }

    if(m_iMaxObjCount <= 0 || m_iObjSize <= 0)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__INVALID_MEM_SIZE);

        return -2;
    }

    m_iUsedCount = 0;
    m_iErrorNO = 0;

    //初始化所有临时内存块，刚开始全部在空闲链中
    unsigned int i;
    for(i = 0; i < (unsigned int)m_iMaxObjCount; ++i)
    {
        m_astTmpMemObj[i].m_iNexIndex = i + 1;
        m_astTmpMemObj[i].m_uiMemOffset = i * (unsigned int)m_iObjSize;
    }

    m_astTmpMemObj[m_iMaxObjCount-1].m_iNexIndex = -1;

    m_iFirstFreeObjIdx = 0;

    return 0;
}

void* CTempMemoryMng::CreateTempMem()
{
    if(m_aucObjMem == NULL || m_iMaxObjCount <= 0 || m_iObjSize <= 0)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__NULL_POINTER);

        return NULL;
    }

    if(m_iUsedCount >= m_iMaxObjCount)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__OBJ_MEMORY_USED_UP);

        return NULL;
    }

    if(m_iFirstFreeObjIdx < 0 || m_iFirstFreeObjIdx >= m_iMaxObjCount)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__OBJ_MEMORY_USED_UP);

        return NULL;
    }

    int iCurUseObjIdx = m_iFirstFreeObjIdx;

    m_iFirstFreeObjIdx = m_astTmpMemObj[m_iFirstFreeObjIdx].m_iNexIndex; //重新设置临时内存队首对象
    ++m_iUsedCount;

    unsigned char* pTempMem = m_aucObjMem + m_astTmpMemObj[iCurUseObjIdx].m_uiMemOffset;

    return pTempMem;
}

int CTempMemoryMng::DestroyTempMem(void *pTempMem)
{
    if(pTempMem == NULL)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__NULL_POINTER);

        return -1;
    }

    if(m_aucObjMem == NULL || m_iMaxObjCount <= 0 || m_iObjSize <= 0)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__NULL_POINTER);

        return -2;
    }

    unsigned int uiOffSet = (unsigned char*)pTempMem - m_aucObjMem;
    int iObjIdx = (int)uiOffSet / m_iObjSize;

    if(iObjIdx < 0 || iObjIdx >= m_iMaxObjCount)
    {
        SetErrorNO(EEN_TEMPMEMORYMNG__INVALID_OBJ_IDX);

        return -3;
    }

    m_astTmpMemObj[iObjIdx].m_iNexIndex = m_iFirstFreeObjIdx;
    m_iFirstFreeObjIdx = iObjIdx;
    --m_iUsedCount;

    return 0;
}

int CTempMemoryMng::GetUsedCount() const
{
    return m_iUsedCount;
}

int CTempMemoryMng::GetFreeCount() const
{
    return (m_iMaxObjCount - m_iUsedCount);
}
