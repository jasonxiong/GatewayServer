/**
*@file BiDirQueue.cpp
*@author jasonxiong
*@date 2009-12-14
*@version 1.0
*@brief CBiDirQueue的实现文件
*
*
*/

#include <assert.h>
#include <stdio.h>

#include "BiDirQueue.hpp"
#include "ErrorDef.hpp"
#include "SharedMemory.hpp"
#include "LogAdapter.hpp"

using namespace ServerLib;

CBiDirQueue* CBiDirQueue::CreateByGivenMemory(char* pszMemoryAddress,
        const size_t uiFreeMemorySize,
        const int iNodeNumber)
{
    if(CountSize(iNodeNumber) > uiFreeMemorySize)
    {
        return NULL;
    }

    CBiDirQueue* pstBiDirQueue = (CBiDirQueue*)pszMemoryAddress;

    if(!pstBiDirQueue)
    {
        return NULL;
    }

    pstBiDirQueue->m_iMaxItemCount = iNodeNumber;
    pstBiDirQueue->m_shQueueAllocType = EBDT_ALLOC_BY_SHARED_MEMORY;
    pstBiDirQueue->m_astQueueItems = (TBiDirQueueItem*)
                                     ((unsigned char*)pszMemoryAddress + sizeof(CBiDirQueue));

    pstBiDirQueue->Initialize();

    return pstBiDirQueue;
}

CBiDirQueue* CBiDirQueue::ResumeByGivenMemory(char* pszMemoryAddress,
        const size_t uiFreeMemorySize,
        const int iNodeNumber)
{
    if(CountSize(iNodeNumber) > uiFreeMemorySize)
    {
        return NULL;
    }

    CBiDirQueue* pstBiDirQueue = (CBiDirQueue*)pszMemoryAddress;

    if(!pstBiDirQueue)
    {
        return NULL;
    }

    pstBiDirQueue->m_iMaxItemCount = iNodeNumber;
    pstBiDirQueue->m_shQueueAllocType = EBDT_ALLOC_BY_SHARED_MEMORY;
    pstBiDirQueue->m_astQueueItems = (TBiDirQueueItem*)
                                     ((unsigned char*)pszMemoryAddress + sizeof(CBiDirQueue));

    return pstBiDirQueue;
}

size_t CBiDirQueue::CountSize(const int iNodeNumber)
{
    size_t uiSize = sizeof(CBiDirQueue) +
                    (unsigned int)iNodeNumber * sizeof(TBiDirQueueItem);

    return uiSize;
}

CBiDirQueue::CBiDirQueue()
{

}

CBiDirQueue::CBiDirQueue(int iMaxItemCount)
{
    __ASSERT_AND_LOG(iMaxItemCount > 0);

    m_astQueueItems = new TBiDirQueueItem[iMaxItemCount];

    __ASSERT_AND_LOG(m_astQueueItems);

    m_shQueueAllocType = EBDT_ALLOC_BY_SELF;
    m_iMaxItemCount = iMaxItemCount;

    Initialize();
}

CBiDirQueue::~CBiDirQueue()
{
    if(m_shQueueAllocType == EBDT_ALLOC_BY_SELF)
    {
        if(m_astQueueItems)
        {
            delete []m_astQueueItems;
            m_astQueueItems = NULL;
        }
    }
}

CBiDirQueue* CBiDirQueue::CreateBySharedMemory(const char* pszKeyFileName,
        const unsigned char ucKeyPrjID,
        int iMaxItemCount)
{
    if(pszKeyFileName == NULL || iMaxItemCount <= 0)
    {
        return NULL;
    }

    CSharedMemory stSharedMemory;
    unsigned int uiSharedMemorySize = sizeof(CBiDirQueue) +
                                      (unsigned int)iMaxItemCount * sizeof(TBiDirQueueItem);

    int iRet = stSharedMemory.CreateShmSegment(pszKeyFileName, ucKeyPrjID,
               (int)uiSharedMemorySize);

    if(iRet)
    {
        return NULL;
    }

    //在共享内存的地址上分配CodeQueue
    CBiDirQueue* pstBiDirQueue = (CBiDirQueue*)stSharedMemory.GetFreeMemoryAddress();

    if(!pstBiDirQueue)
    {
        return NULL;
    }

    pstBiDirQueue->m_iMaxItemCount = iMaxItemCount;
    pstBiDirQueue->m_shQueueAllocType = EBDT_ALLOC_BY_SHARED_MEMORY;
    pstBiDirQueue->m_astQueueItems = (TBiDirQueueItem*)((unsigned char*)stSharedMemory.GetFreeMemoryAddress() +
                                     sizeof(CBiDirQueue));

    return pstBiDirQueue;
}

int CBiDirQueue::Initialize()
{
    m_iErrorNO = 0;
    m_iHeadIdx = -1;
    m_iTailIdx = -1;
    m_iCurItemCount = 0;

    int i;
    for(i = 0; i < m_iMaxItemCount; ++i)
    {
        m_astQueueItems[i].m_iNextItem = -1;
        m_astQueueItems[i].m_iPreItem = -1;
        m_astQueueItems[i].m_cUseFlag = EINF_NOT_USED;
    }

    return 0;
}

int CBiDirQueue::GetNextItem(int iItemIdx, int& iNextItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //该节点未被使用
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_NOT_USED)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__ITEM_NOT_BE_USED);

        return -3;
    }

    iNextItemIdx = m_astQueueItems[iItemIdx].m_iNextItem;

    return 0;
}

int CBiDirQueue::GetPrevItem(int iItemIdx, int& iPrevItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //该节点未被使用
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_NOT_USED)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__ITEM_NOT_BE_USED);

        return -3;
    }

    iPrevItemIdx = m_astQueueItems[iItemIdx].m_iPreItem;

    return 0;
}

int CBiDirQueue::SetNextItem(int iItemIdx, int iNextItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    m_astQueueItems[iItemIdx].m_iNextItem = iNextItemIdx;

    return 0;
}

int CBiDirQueue::SetPrevItem(int iItemIdx, int iPrevItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    m_astQueueItems[iItemIdx].m_iPreItem = iPrevItemIdx;

    return 0;
}

int CBiDirQueue::DeleteItem(int iItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //该节点未被使用则直接返回成功
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_NOT_USED)
    {
        m_astQueueItems[iItemIdx].m_iPreItem = -1;
        m_astQueueItems[iItemIdx].m_iNextItem = -1;

        return 0;
    }

    int iPrevItemIdx = m_astQueueItems[iItemIdx].m_iPreItem;
    int iNextItemIdx = m_astQueueItems[iItemIdx].m_iNextItem;

    if(iPrevItemIdx == -1)
    {
        //前延节点为空，则被删除节点必为首节点
        if(iItemIdx != m_iHeadIdx)
        {
            SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

            return -3;
        }

        m_iHeadIdx = iNextItemIdx;
    }
    else
    {
        SetNextItem(iPrevItemIdx, iNextItemIdx);
    }

    if(iNextItemIdx == -1)
    {
        //后续节点为空，则被删除节点必为尾节点
        if(iItemIdx != m_iTailIdx)
        {
            SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

            return -4;
        }

        m_iTailIdx = iPrevItemIdx;
    }
    else
    {
        SetPrevItem(iNextItemIdx, iPrevItemIdx);
    }

    m_astQueueItems[iItemIdx].m_iPreItem = -1;
    m_astQueueItems[iItemIdx].m_iNextItem = -1;
    m_astQueueItems[iItemIdx].m_cUseFlag = EINF_NOT_USED;

    --m_iCurItemCount;

    return 0;
}

int CBiDirQueue::AppendItemToTail(int iItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //已经是最后一个节点了，则直接返回
    if(m_iTailIdx == iItemIdx)
    {
        return 0;
    }

    //如果该节点已经在队列中，则需要先从队列中删除
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_USED)
    {
        DeleteItem(iItemIdx);
    }

    m_astQueueItems[iItemIdx].m_iNextItem = -1;
    m_astQueueItems[iItemIdx].m_iPreItem = m_iTailIdx;
    m_astQueueItems[iItemIdx].m_cUseFlag = EINF_USED;
    ++m_iCurItemCount;

    if(m_iTailIdx == -1)
    {
        m_iTailIdx = m_iHeadIdx = iItemIdx;
    }
    else
    {
        SetNextItem(m_iTailIdx, iItemIdx);

        m_iTailIdx = iItemIdx;
    }

    return 0;
}

int CBiDirQueue::PopHeadItem()
{
    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -1;
    }

    int iTempItem = m_iHeadIdx;

    if(m_iHeadIdx != -1)
    {
        DeleteItem(m_iHeadIdx);
    }

    return iTempItem;
}

int CBiDirQueue::PopTailItem()
{
    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -1;
    }

    int iTempItem = m_iHeadIdx;

    if(m_iTailIdx != -1)
    {
        DeleteItem(m_iTailIdx);
    }

    return iTempItem;
}

int CBiDirQueue::InsertItemToHead(int iItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //已经是第一个节点了，则直接返回
    if(m_iHeadIdx == iItemIdx)
    {
        return 0;
    }

    //如果该节点已经在队列中，则需要先从队列中删除
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_USED)
    {
        DeleteItem(iItemIdx);
    }

    m_astQueueItems[iItemIdx].m_iPreItem = -1;
    m_astQueueItems[iItemIdx].m_iNextItem = m_iHeadIdx;
    m_astQueueItems[iItemIdx].m_cUseFlag = EINF_USED;
    ++m_iCurItemCount;

    if(m_iHeadIdx == -1)
    {
        m_iTailIdx = m_iHeadIdx = iItemIdx;
    }
    else
    {
        SetPrevItem(m_iHeadIdx, iItemIdx);

        m_iHeadIdx = iItemIdx;
    }

    return 0;
}

int CBiDirQueue::InsertItemAfter(int iItemIdx, int iPrevItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount ||
            iPrevItemIdx < 0 || iPrevItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //两个节点索引相同，
    if(iItemIdx == iPrevItemIdx)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INSERT_SAME_INDEX);

        return -3;
    }

    //已经处于正确的位置，直接返回
    if(m_astQueueItems[iPrevItemIdx].m_iNextItem == iItemIdx)
    {
        return 0;
    }

    //如果该节点已经在队列中，则需要先从队列中删除
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_USED)
    {
        DeleteItem(iItemIdx);
    }

    int iNextItemIdx = m_astQueueItems[iPrevItemIdx].m_iNextItem;

    if(iNextItemIdx == -1)
    {
        //若插入位置没有后续节点，则必为尾节点
        if(iPrevItemIdx != m_iTailIdx)
        {
            SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

            return -4;
        }

        return AppendItemToTail(iItemIdx);
    }

    //插入节点的后续节点即iPrevItemIdx的后续节点
    m_astQueueItems[iItemIdx].m_iNextItem = iNextItemIdx;
    //插入节点的前延节点即iPrevItemIdx
    m_astQueueItems[iItemIdx].m_iPreItem = iPrevItemIdx;

    //前延节点的后续节点修改为插入节点
    m_astQueueItems[iPrevItemIdx].m_iNextItem = iItemIdx;
    //后续节点的前延节点修改为插入节点
    m_astQueueItems[iNextItemIdx].m_iPreItem = iItemIdx;

    m_astQueueItems[iItemIdx].m_cUseFlag = EINF_USED;

    ++m_iCurItemCount;

    return 0;
}

int CBiDirQueue::InsertItemBefore(int iItemIdx, int iNextItemIdx)
{
    if(iItemIdx < 0 || iItemIdx >= m_iMaxItemCount ||
            iNextItemIdx < 0 || iNextItemIdx >= m_iMaxItemCount)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

        return -1;
    }

    if(m_astQueueItems == NULL)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__NULL_POINTER);

        return -2;
    }

    //两个节点索引相同，
    if(iItemIdx == iNextItemIdx)
    {
        SetErrorNO(EEN_BI_DIR_QUEUE__INSERT_SAME_INDEX);

        return -3;
    }

    //已经处于正确的位置，直接返回
    if(m_astQueueItems[iNextItemIdx].m_iPreItem == iItemIdx)
    {
        return 0;
    }

    //如果该节点已经在队列中，则需要先从队列中删除
    if(m_astQueueItems[iItemIdx].m_cUseFlag == EINF_USED)
    {
        DeleteItem(iItemIdx);
    }

    int iPrevItemIdx = m_astQueueItems[iNextItemIdx].m_iPreItem;

    //若插入位置无前一个节点，则必为头节点
    if(iPrevItemIdx == -1)
    {
        if(iNextItemIdx != m_iHeadIdx)
        {
            SetErrorNO(EEN_BI_DIR_QUEUE__INVALID_ITEM_IDX);

            return -4;
        }

        return InsertItemToHead(iItemIdx);
    }

    //插入节点的下一个节点即iNextItemIdx
    m_astQueueItems[iItemIdx].m_iNextItem = iNextItemIdx;
    //插入节点的前一个节点即iNextItemIdx的前一个节点
    m_astQueueItems[iItemIdx].m_iPreItem = iPrevItemIdx;

    //iNextItemIdx节点的前一个节点修改为插入节点，后一个节点保持不变
    m_astQueueItems[iNextItemIdx].m_iPreItem = iItemIdx;
    //iPrevItemIdx节点的后一个节点修改为插入节点，前一个节点保持不变
    m_astQueueItems[iPrevItemIdx].m_iNextItem = iItemIdx;

    m_astQueueItems[iItemIdx].m_cUseFlag = EINF_USED;

    ++m_iCurItemCount;

    return 0;
}
