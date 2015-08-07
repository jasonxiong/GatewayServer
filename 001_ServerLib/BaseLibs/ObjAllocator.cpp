/**
*@file ObjAllocator.cpp
*@author jasonxiong
*@date 2009-12-14
*@version 1.0
*@brief 对象分配类的实现文件
*
*
*/

#include <assert.h>

#include "ErrorDef.hpp"
#include "ObjAllocator.hpp"
#include "SharedMemory.hpp"

using namespace ServerLib;

CIdx::CIdx()
{
    Initialize();
}

CIdx::~CIdx()
{

}

int CIdx::Initialize()
{
    m_iNextIdx = -1;
    m_iPrevIdx = -1;
    m_iUseFlag = 0;
    m_pAttachedObj = NULL;

    return 0;
}

CObjAllocator::CObjAllocator()
{

}

//void* CObjAllocator::operator new(unsigned int uiSize, const void* pThis) throw()
//{
//	if(!pThis)
//	{
//		return NULL;
//	}
//
//	return (void*)pThis;
//}

CObjAllocator::CObjAllocator(size_t uiObjSize, int iObjCount, Function_CreateObj pfCreateObj)
{
    __ASSERT_AND_LOG(uiObjSize > 0 && iObjCount > 0 && pfCreateObj);

    m_shObjAllocType = EOAT_ALLOC_BY_SELF;
    m_iObjCount = iObjCount;
    m_uiObjSize = uiObjSize;
    m_pfCreateObjFunc = pfCreateObj;

    m_astIdxs = new CIdx[m_iObjCount];
    size_t uiObjMemorySize = uiObjSize * iObjCount;
    char* pstObjMem = new char[uiObjMemorySize];
    m_pstObjBuffer = (CObj*)pstObjMem;

    __ASSERT_AND_LOG(m_astIdxs && m_pstObjBuffer);

    Initialize();
}

size_t CObjAllocator::CountSize(size_t uiObjSize, int iObjCount)
{
    return sizeof(CObjAllocator) + uiObjSize * iObjCount + iObjCount * sizeof(CIdx);
}

CObjAllocator* CObjAllocator::CreateByGivenMemory(char* pszMemoryAddress, size_t uiMemorySize, size_t uiObjSize,
        int iObjCount, Function_CreateObj pfCreateObj)
{
    if(pszMemoryAddress == NULL || uiObjSize <= 0 || iObjCount <= 0 || pfCreateObj == NULL)
    {
        TRACESVR("%p, %d, %d, %p.\n", pszMemoryAddress, (int)uiObjSize, iObjCount, pfCreateObj);
        return NULL;
    }

    size_t uiSharedMemorySize = sizeof(CObjAllocator) + uiObjSize * iObjCount +
                                iObjCount * sizeof(CIdx);

    if(uiSharedMemorySize > uiMemorySize)
    {
        TRACESVR("ObjAllocator: alloc size %lu > sh size %lu.\n", (unsigned long)uiSharedMemorySize, (unsigned long)uiMemorySize);
        return NULL;
    }

    //在指定的内存地址上分配CObjAllocator
    CObjAllocator* pstObjAllocator = (CObjAllocator*)pszMemoryAddress;

    if(!pstObjAllocator)
    {
        TRACESVR("ObjAllocator: pstObjAllocator is NULL.\n");
        return NULL;
    }

    pstObjAllocator->m_uiObjSize = uiObjSize;
    pstObjAllocator->m_iObjCount = iObjCount;
    pstObjAllocator->m_pfCreateObjFunc = pfCreateObj;
    pstObjAllocator->m_shObjAllocType = EOAT_ALLOC_BY_SHARED_MEMORY;
    pstObjAllocator->m_astIdxs = (CIdx*)((unsigned char*)pszMemoryAddress + sizeof(CObjAllocator));
    pstObjAllocator->m_pstObjBuffer = (CObj*)((unsigned char*)pszMemoryAddress + sizeof(CObjAllocator) + iObjCount * sizeof(CIdx));

    pstObjAllocator->Initialize();

    return pstObjAllocator;
}

CObjAllocator* CObjAllocator::ResumeByGivenMemory(char* pszMemoryAddress,
        size_t uiMemorySize, size_t uiObjSize, int iObjCount, Function_CreateObj pfCreateObj)
{
    if ((NULL == pszMemoryAddress) || (uiObjSize <= 0) || (iObjCount <= 0))
    {
        return NULL;
    }

    size_t uiSharedMemorySize = sizeof(CObjAllocator)
                                + uiObjSize * iObjCount + sizeof(CIdx) * iObjCount;

    if (uiSharedMemorySize > uiMemorySize)
    {
        return NULL;
    }

    CObjAllocator* pstObjAllocator = (CObjAllocator*) pszMemoryAddress;

    if ((pstObjAllocator->m_uiObjSize != uiObjSize)
            || (pstObjAllocator->m_iObjCount != iObjCount))
    {
        return NULL;
    }

    pstObjAllocator->m_shObjAllocType = EOAT_ALLOC_BY_SHARED_MEMORY;
    pstObjAllocator->m_astIdxs = (CIdx*)((unsigned char*)pszMemoryAddress + sizeof(CObjAllocator));
    pstObjAllocator->m_pstObjBuffer = (CObj*)((unsigned char*)pszMemoryAddress + sizeof(CObjAllocator) + iObjCount * sizeof(CIdx));

    int i;

    // 重新绑定obj和idx
    for (i = 0; i < iObjCount; ++i)
    {
        // 调用placement-new, 恢复类的虚函数表.
        CObj* pstObj = (*pfCreateObj)((unsigned char*)pstObjAllocator->m_pstObjBuffer + uiObjSize * i);

        __ASSERT_AND_LOG(pstObj->GetObjectID() == i);

        pstObjAllocator->m_astIdxs[i].SetAttachedObj(pstObj);
    }

    pstObjAllocator->SetCreateObjFunc(pfCreateObj);
    return pstObjAllocator;
}

CObjAllocator* CObjAllocator::CreateBySharedMemory(const char* pszKeyFileName, const unsigned char ucKeyPrjID,
        size_t uiObjSize, int iObjCount, Function_CreateObj pfCreateObj)
{
    if(pszKeyFileName == NULL || uiObjSize <= 0 || iObjCount <= 0 || pfCreateObj == NULL)
    {
        return NULL;
    }

    CSharedMemory stSharedMemory;
    size_t uiSharedMemorySize = sizeof(CObjAllocator) + uiObjSize * iObjCount +
                                iObjCount * sizeof(CIdx);

    int iRet = stSharedMemory.CreateShmSegment(pszKeyFileName, ucKeyPrjID,
               (int)uiSharedMemorySize);

    if(iRet)
    {
        return NULL;
    }

    //在共享内存的地址上分配CObjAllocator
    CObjAllocator* pstObjAllocator = (CObjAllocator*)stSharedMemory.GetFreeMemoryAddress();

    if(!pstObjAllocator)
    {
        return NULL;
    }

    pstObjAllocator->m_uiObjSize = uiObjSize;
    pstObjAllocator->m_iObjCount = iObjCount;
    pstObjAllocator->m_pfCreateObjFunc = pfCreateObj;
    pstObjAllocator->m_shObjAllocType = EOAT_ALLOC_BY_SHARED_MEMORY;
    pstObjAllocator->m_astIdxs = (CIdx*)((unsigned char*)stSharedMemory.GetFreeMemoryAddress() + sizeof(CObjAllocator));
    pstObjAllocator->m_pstObjBuffer = (CObj*)((unsigned char*)stSharedMemory.GetFreeMemoryAddress() +
                                      sizeof(CObjAllocator) + iObjCount * sizeof(CIdx));

    return pstObjAllocator;
}

CObjAllocator::~CObjAllocator()
{
    if(m_shObjAllocType == EOAT_ALLOC_BY_SELF)
    {
        if(m_astIdxs)
        {
            delete []m_astIdxs;
            m_astIdxs = NULL;
        }

        if(m_pstObjBuffer)
        {
            char* pstObjMem = (char*)m_pstObjBuffer;
            delete []pstObjMem;
            m_pstObjBuffer = NULL;
        }
    }
}

int CObjAllocator::Initialize()
{
    if(m_pstObjBuffer == NULL || m_astIdxs == NULL)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);

        return -1;
    }

    if(m_iObjCount <= 0)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_COUNT);

        return -2;
    }

    //初始化索引数组
    int i;
    for(i = 0; i < m_iObjCount; ++i)
    {
        m_astIdxs[i].Initialize();
        m_astIdxs[i].SetPrevIdx(i-1);
        m_astIdxs[i].SetNextIdx(i+1);
    }

    m_astIdxs[m_iObjCount-1].SetNextIdx(-1);

    //初始化对象数组
    for(i = 0; i < m_iObjCount; ++i)
    {
        CObj* pstObj = (*m_pfCreateObjFunc)((unsigned char*)m_pstObjBuffer + m_uiObjSize * i);
        pstObj->m_iObjectID = i;
        m_astIdxs[i].SetAttachedObj(pstObj);
    }

    m_iErrorNO = 0;
    m_iFreeHeadIdx = 0;
    m_iFreeTailIdx = m_iObjCount-1;
    m_iUsedHeadIdx = -1;
    m_iUsedCount = 0;

    return 0;
}

int CObjAllocator::CreateObject()
{
    if(m_pstObjBuffer == NULL || m_astIdxs == NULL)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);

        return -1;
    }

    if(m_iUsedCount >= m_iObjCount)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__OBJ_IS_FULL);

        return -2;
    }

    if(m_iFreeHeadIdx < 0 || m_iFreeHeadIdx >= m_iObjCount)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_INDEX);

        return -3;
    }

    //修改空闲链表
    int iCurIdx = m_iFreeHeadIdx;
    m_iFreeHeadIdx = m_astIdxs[m_iFreeHeadIdx].GetNextIdx();

    if(m_iFreeHeadIdx >= 0)
    {
        m_astIdxs[m_iFreeHeadIdx].SetPrevIdx(-1);
    }

    if (iCurIdx == m_iFreeTailIdx)
    {
        m_iFreeTailIdx = -1;
    }

    //挂到使用链表
    m_astIdxs[iCurIdx].SetUsed();
    m_astIdxs[iCurIdx].SetNextIdx(m_iUsedHeadIdx);
    m_astIdxs[iCurIdx].SetPrevIdx(-1);

    if(m_iUsedHeadIdx >= 0)
    {
        m_astIdxs[m_iUsedHeadIdx].SetPrevIdx(iCurIdx);
    }

    //初始化对象
    m_iUsedHeadIdx = iCurIdx;

    CObj* pstObj = m_astIdxs[iCurIdx].GetAttachedObj();
    if (NULL == pstObj)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);
        return -4;
    }

#ifdef OBJ_MEMSET_ON_CREATE
    memset(pstObj, 0, m_uiObjSize);
    (*m_pfCreateObjFunc)((unsigned char*)pstObj);
    pstObj->SetObjectID(iCurIdx);
#endif

    pstObj->Initialize();
    ++m_iUsedCount;

    __ASSERT_AND_LOG(pstObj->GetObjectID() == iCurIdx);

    return iCurIdx;
}

int	CObjAllocator::CreateObjectByID(int iID)
{
    if(m_pstObjBuffer == NULL || m_astIdxs == NULL)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);

        return -1;
    }

    if (iID < 0 || iID >= m_iObjCount)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_INDEX);

        return -2;
    }

    if (m_astIdxs[iID].IsUsed())
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_INDEX);

        return -3;
    }

    //修改空闲链表
    int iCurIdx = iID;
    int iPrevIdx = m_astIdxs[iCurIdx].GetPrevIdx();
    int iNextIdx = m_astIdxs[iCurIdx].GetNextIdx();

    if (iPrevIdx >= 0)
    {
        m_astIdxs[iPrevIdx].SetNextIdx(iNextIdx);
    }

    if (iNextIdx >= 0)
    {
        m_astIdxs[iNextIdx].SetPrevIdx(iPrevIdx);
    }

    if (iCurIdx == m_iFreeHeadIdx)
    {
        m_iFreeHeadIdx = iNextIdx;
    }

    if (iCurIdx == m_iFreeTailIdx)
    {
        m_iFreeTailIdx = -1;
    }

    //挂到使用链表
    m_astIdxs[iCurIdx].SetUsed();
    m_astIdxs[iCurIdx].SetNextIdx(m_iUsedHeadIdx);
    m_astIdxs[iCurIdx].SetPrevIdx(-1);

    if(m_iUsedHeadIdx >= 0)
    {
        m_astIdxs[m_iUsedHeadIdx].SetPrevIdx(iCurIdx);
    }

    m_iUsedHeadIdx = iCurIdx;  // add by cary

    CObj* pstObj = m_astIdxs[iCurIdx].GetAttachedObj();

#ifdef OBJ_MEMSET_ON_CREATE
    memset(pstObj, 0, m_uiObjSize);
    (*m_pfCreateObjFunc)((unsigned char*)pstObj);
    pstObj->SetObjectID(iCurIdx);
#endif

    pstObj->Initialize();
    ++m_iUsedCount;

    __ASSERT_AND_LOG(pstObj->GetObjectID() == iCurIdx);

    return iCurIdx;
}

int CObjAllocator::DestroyObject(int iObjectID)
{
    if(m_pstObjBuffer == NULL || m_astIdxs == NULL)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);

        return -1;
    }

    if(iObjectID >= m_iObjCount || iObjectID < 0 || m_iObjCount <= 0)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_INDEX);

        return -2;
    }

    if(!m_astIdxs[iObjectID].IsUsed())
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__DESTROY_FREE_OBJ);

        return -3;
    }


    //从已用链表中删除
    int iCurIdx = iObjectID;
    int iPrevIdx = m_astIdxs[iCurIdx].GetPrevIdx();
    int iNextIdx = m_astIdxs[iCurIdx].GetNextIdx();

    if (iPrevIdx >= 0)
    {
        m_astIdxs[iPrevIdx].SetNextIdx(iNextIdx);
    }

    if (iNextIdx >= 0)
    {
        m_astIdxs[iNextIdx].SetPrevIdx(iPrevIdx);
    }

    if(iCurIdx == m_iUsedHeadIdx)
    {
        m_iUsedHeadIdx = iNextIdx;
    }

    //挂入空闲链表尾部
    m_astIdxs[iObjectID].SetFree();
    m_astIdxs[iObjectID].SetPrevIdx(m_iFreeTailIdx);
    m_astIdxs[iObjectID].SetNextIdx(-1);

    if (m_iFreeHeadIdx == -1)
    {
        m_iFreeHeadIdx = iCurIdx;
    }

    if(m_iFreeTailIdx >= 0)
    {
        m_astIdxs[m_iFreeTailIdx].SetNextIdx(iCurIdx);
    }

    m_iFreeTailIdx = iCurIdx;
    --m_iUsedCount;

    return 0;
}

CObj* CObjAllocator::GetObj(int iObjectID)
{
    if(m_pstObjBuffer == NULL || m_astIdxs == NULL)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);

        return NULL;
    }

    if(iObjectID < 0 || iObjectID >= m_iObjCount)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_INDEX);

        return NULL;
    }

    if(!m_astIdxs[iObjectID].IsUsed())
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__GET_FREE_OBJ);

        return NULL;
    }

    return m_astIdxs[iObjectID].GetAttachedObj();
}

CIdx* CObjAllocator::GetIdx(int iObjectID)
{
    if(m_pstObjBuffer == NULL || m_astIdxs == NULL)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__NULL_POINTER);

        return NULL;
    }

    if(iObjectID < 0 || iObjectID >= m_iObjCount)
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__INVALID_OBJ_INDEX);

        return NULL;
    }

    if(!m_astIdxs[iObjectID].IsUsed())
    {
        SetErrorNO(EEN_OBJ_ALLOCATOR__GET_FREE_OBJ);

        return NULL;
    }

    return &m_astIdxs[iObjectID];
}

CObj* CObjAllocator::GetNextObj(int iObjectID)
{
    CIdx* pIdx = GetIdx(iObjectID);

    if (!pIdx)
    {
        return NULL;
    }

    int iNextObjIdx = pIdx->GetNextIdx();

    return GetObj(iNextObjIdx);
}
