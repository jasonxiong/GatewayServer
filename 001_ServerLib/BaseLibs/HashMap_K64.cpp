
#include "HashMap_K64.hpp"

#include "LogAdapter.hpp"


using namespace ServerLib;



CHashMap_K64* CHashMap_K64::CreateHashMap(char* pszMemoryAddress,
        const unsigned int uiFreeMemorySize,
        const int iNodeNumber)
{
    if(CountSize(iNodeNumber) > uiFreeMemorySize)
    {
        return NULL;
    }

    return new(pszMemoryAddress) CHashMap_K64(iNodeNumber);
}

CHashMap_K64* CHashMap_K64::ResumeHashMap(char* pszMemoryAddress,
        const unsigned int uiFreeMemorySize, const int iNodeNumber)
{
    if((NULL == pszMemoryAddress) || (CountSize(iNodeNumber) > uiFreeMemorySize))
    {
        return NULL;
    }

    CHashMap_K64* pHashMap = (CHashMap_K64*) pszMemoryAddress;

    if (pHashMap->GetNodeSize() != iNodeNumber)
    {
        return NULL;
    }

    pHashMap->m_pastHashNode = (THashMapNode_K64*) ((char*) pHashMap + sizeof(CHashMap_K64));
    pHashMap->m_paiHashFirstIndex = (int*)((char*)(pHashMap->m_pastHashNode) + sizeof(THashMapNode_K64) * iNodeNumber);

    return pHashMap;
}

size_t CHashMap_K64::CountSize(const int iNodeNumber)
{
    return sizeof(CHashMap_K64) + (sizeof(THashMapNode_K64) + sizeof(int)) * iNodeNumber;
}

CHashMap_K64::CHashMap_K64()
{
}

CHashMap_K64::CHashMap_K64(int iNodeNumber)
{
    __ASSERT_AND_LOG(iNodeNumber > 0);

    m_iErrorNO = 0;

    m_iNodeNumber = iNodeNumber;

    m_pastHashNode = (THashMapNode_K64*)((char*)(this) + sizeof(CHashMap_K64));
    m_paiHashFirstIndex = (int*)((char*)(m_pastHashNode) + sizeof(THashMapNode_K64) * iNodeNumber);

    EraseAll();
}

CHashMap_K64::~CHashMap_K64()
{
}

int CHashMap_K64::EraseAll()
{
    m_iUsedNodeNumber = 0;
    m_iFirstFreeIndex = 0;

    int i;
    for(i = 0; i < m_iNodeNumber; ++i)
    {
        m_pastHashNode[i].m_iHashNext = i + 1;
        m_pastHashNode[i].m_iIsNodeUsed = EHNS_K64_FREE;
    }

    m_pastHashNode[m_iNodeNumber-1].m_iHashNext = -1;

    for(i = 0; i < m_iNodeNumber; ++i)
    {
        m_paiHashFirstIndex[i] = -1;
    }

    return 0;
}

int CHashMap_K64::GetValueByIndex(const int iNodeIndex, int& riValue)
{
    if(iNodeIndex < 0 || iNodeIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    if(!m_pastHashNode[iNodeIndex].m_iIsNodeUsed)
    {
        return -2;
    }

    riValue = m_pastHashNode[iNodeIndex].m_iValue;

    return 0;
}

int CHashMap_K64::GetValueByKey(const TData64 stPriKey, int& riValue)
{
    int iHashIndex = HashKeyToIndex(stPriKey);

    if(iHashIndex < 0 || iHashIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    int iCurrentIndex = m_paiHashFirstIndex[iHashIndex];

    while(iCurrentIndex != -1)
    {
        //if(stPriKey == m_pastHashNode[iCurrentIndex].m_stPriKey)
        if(CompareKey(stPriKey,m_pastHashNode[iCurrentIndex].m_stPriKey))
        {
            riValue = m_pastHashNode[iCurrentIndex].m_iValue;
            break;
        }

        iCurrentIndex = m_pastHashNode[iCurrentIndex].m_iHashNext;
    }

    if(iCurrentIndex == -1)
    {
        return -2;
    }

    return 0;
}

int* CHashMap_K64::GetValuePtrByKey(const TData64 stPriKey)
{
    int iHashIndex = HashKeyToIndex(stPriKey);

    if(iHashIndex < 0 || iHashIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return NULL;
    }

    int iCurrentIndex = m_paiHashFirstIndex[iHashIndex];
    int* piValue = NULL;

    while(iCurrentIndex != -1)
    {
        //if(stPriKey == m_pastHashNode[iCurrentIndex].m_stPriKey)
        if(CompareKey(stPriKey,m_pastHashNode[iCurrentIndex].m_stPriKey))
        {
            piValue = &m_pastHashNode[iCurrentIndex].m_iValue;
            break;
        }

        iCurrentIndex = m_pastHashNode[iCurrentIndex].m_iHashNext;
    }

    if(iCurrentIndex == -1)
    {
        return NULL;
    }

    return piValue;
}

int CHashMap_K64::DeleteByKey(const TData64 stPriKey, int& riValue)
{
    int iHashIndex = HashKeyToIndex(stPriKey);

    if(iHashIndex < 0 || iHashIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    int iPreIndex = -1;
    int iCurrentIndex = m_paiHashFirstIndex[iHashIndex];

    while(iCurrentIndex != -1)
    {
        //已经存在该Key则返回失败
        //if(stPriKey == m_pastHashNode[iCurrentIndex].m_stPriKey)
        if(CompareKey(stPriKey,m_pastHashNode[iCurrentIndex].m_stPriKey))
        {
            break;
        }

        iPreIndex = iCurrentIndex;
        iCurrentIndex = m_pastHashNode[iCurrentIndex].m_iHashNext;
    }

    //找到需要删除的节点
    if(iCurrentIndex != -1)
    {
        //是冲突链表头节点
        if(m_paiHashFirstIndex[iHashIndex] == iCurrentIndex)
        {
            m_paiHashFirstIndex[iHashIndex] = m_pastHashNode[iCurrentIndex].m_iHashNext;
        }
        //将上一个节点的Next索引等于当前要删除节点的Next索引
        else
        {
            m_pastHashNode[iPreIndex].m_iHashNext = m_pastHashNode[iCurrentIndex].m_iHashNext;
        }

        riValue = m_pastHashNode[iCurrentIndex].m_iValue;

        m_pastHashNode[iCurrentIndex].m_iIsNodeUsed = EHNS_K64_FREE;
        m_pastHashNode[iCurrentIndex].m_iHashNext = m_iFirstFreeIndex;
        m_iFirstFreeIndex = iCurrentIndex;
        --m_iUsedNodeNumber;
    }
    else
    {
        // 未找到，返回失败
        SetErrorNO(EEN_HASH_MAP__NODE_NOT_EXISTED);
        return -2;
    }

    return 0;
}

int CHashMap_K64::InsertValueByKey(const TData64 stPriKey, const int iValue)
{
    int iHashIndex = HashKeyToIndex(stPriKey);

    if(iHashIndex < 0 || iHashIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    //已经没有可用的节点了
    if(m_iFirstFreeIndex < 0)
    {
        SetErrorNO(EEN_HASH_MAP__NODE_IS_FULL);

        return -2;
    }

    int iPreIndex = -1;
    int iCurrentIndex = m_paiHashFirstIndex[iHashIndex];

    while(iCurrentIndex != -1)
    {
        //已经存在该Key则返回失败
        //if(stPriKey == m_pastHashNode[iCurrentIndex].m_stPriKey)
        if(CompareKey(stPriKey,m_pastHashNode[iCurrentIndex].m_stPriKey))
        {
            SetErrorNO(EEN_HASH_MAP__INSERT_FAILED_FOR_KEY_DUPLICATE);

            return -3;
        }

        iPreIndex = iCurrentIndex;
        iCurrentIndex = m_pastHashNode[iCurrentIndex].m_iHashNext;
    }

    int iNowAssignIdx = m_iFirstFreeIndex;
    m_iFirstFreeIndex = m_pastHashNode[m_iFirstFreeIndex].m_iHashNext;
    ++m_iUsedNodeNumber;
    m_pastHashNode[iNowAssignIdx].m_stPriKey = stPriKey;
    m_pastHashNode[iNowAssignIdx].m_iValue = iValue;
    m_pastHashNode[iNowAssignIdx].m_iIsNodeUsed = EHNS_K64_USED;
    m_pastHashNode[iNowAssignIdx].m_iHashNext = -1;

    //是冲突链表的第一个节点
    if(m_paiHashFirstIndex[iHashIndex] == -1)
    {
        m_paiHashFirstIndex[iHashIndex] = iNowAssignIdx;
    }
    else
    {
        m_pastHashNode[iPreIndex].m_iHashNext = iNowAssignIdx;
    }

    return 0;
}

int CHashMap_K64::UpdateValueByKey(const TData64 stPriKey, const int iValue)
{
    int* piValue = GetValuePtrByKey(stPriKey);
    if(!piValue)
    {
        SetErrorNO(EEN_HASH_MAP__NODE_NOT_EXISTED);
        return -1;
    }

    *piValue = iValue;

    return 0;
}

int CHashMap_K64::ReplaceValueByKey(const TData64 stPriKey, const int iValue)
{
    int iHashIndex = HashKeyToIndex(stPriKey);

    if(iHashIndex < 0 || iHashIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    int iPreIndex = -1;
    int iCurrentIndex = m_paiHashFirstIndex[iHashIndex];

    while(iCurrentIndex != -1)
    {
        //已经存在该Key则直接更新
        //if(stPriKey == m_pastHashNode[iCurrentIndex].m_stPriKey)
        if(CompareKey(stPriKey,m_pastHashNode[iCurrentIndex].m_stPriKey))
        {
            m_pastHashNode[iCurrentIndex].m_iValue = iValue;
            return 0;
        }

        iPreIndex = iCurrentIndex;
        iCurrentIndex = m_pastHashNode[iCurrentIndex].m_iHashNext;
    }

    //已经没有可用的节点了
    if(m_iFirstFreeIndex < 0)
    {
        SetErrorNO(EEN_HASH_MAP__NODE_IS_FULL);

        return -2;
    }

    int iNowAssignIdx = m_iFirstFreeIndex;
    m_iFirstFreeIndex = m_pastHashNode[m_iFirstFreeIndex].m_iHashNext;
    ++m_iUsedNodeNumber;
    m_pastHashNode[iNowAssignIdx].m_stPriKey = stPriKey;
    m_pastHashNode[iNowAssignIdx].m_iValue = iValue;
    m_pastHashNode[iNowAssignIdx].m_iIsNodeUsed = EHNS_K64_USED;
    m_pastHashNode[iNowAssignIdx].m_iHashNext = -1;

    //是冲突链表的第一个节点
    if(m_paiHashFirstIndex[iHashIndex] == -1)
    {
        m_paiHashFirstIndex[iHashIndex] = iNowAssignIdx;
    }
    else
    {
        m_pastHashNode[iPreIndex].m_iHashNext = iNowAssignIdx;
    }

    return 0;
}

int CHashMap_K64::HashKeyToIndex(const TData64 stPriKey) const
{
    int hash_index=(int)(((((stPriKey.m_uiKey1)>>16)+(((stPriKey.m_uiKey1)<<16)>>16)) * (((stPriKey.m_uiKey2)>>16)+(((stPriKey.m_uiKey2)<<16)>>16)))% m_iNodeNumber);
    //return (int)(((uiKey>>16) + (uiKey<<16) >> 16) % m_iNodeNumber);
    return hash_index;
}

int CHashMap_K64::CompareKey(const TData64& key1,const TData64& key2)
{
    return ((key1.m_uiKey1==key2.m_uiKey1) && (key1.m_uiKey2==key2.m_uiKey2));

}


//
//#include <stdio.h>
//
//int main()
//{
//  printf("CountSize %d \n", CHashMap_K64::CountSize(0));
//  printf("CountSize %d \n", CHashMap_K64::CountSize(1));
//  printf("CountSize %d \n", CHashMap_K64::CountSize(2));
//  printf("CountSize %d \n", CHashMap_K64::CountSize(10));
//  printf("CountSize %d \n", CHashMap_K64::CountSize(100));
//  printf("CountSize %d \n", CHashMap_K64::CountSize(100000));
//
//  char* pszBuffer = new char[2500000];
//  CHashMap_K64* pstIntHash = new(pszBuffer) CHashMap_K64(100000);
//
//  pstIntHash->InsertValueByKey(32186690, 10000);
//  pstIntHash->ReplaceValueByKey(10086, 10);
//  pstIntHash->ReplaceValueByKey(32186690, 20000);
//
//  printf("UsedNum = %d\n", pstIntHash->GetUsedNodeNumber());
//
//  int* v1 = pstIntHash->GetValuePtrByKey(32186690);
//
//  if(v1 == NULL)
//  {
//      printf("v1 == NULL\n");
//
//      return -1;
//  }
//
//  printf("v1 = %d\n", *v1);
//
//  pstIntHash->UpdateValueByKey(32186690, 10000);
//
//  v1 = pstIntHash->GetValuePtrByKey(32186690);
//
//  if(v1 == NULL)
//  {
//      printf("v1 == NULL\n");
//
//      return -1;
//  }
//
//  printf("v1 = %d\n", *v1);
//
//  int* v2 = pstIntHash->GetValuePtrByKey(10086);
//
//  if(v2 == NULL)
//  {
//      printf("v2 == NULL\n");
//
//      return -1;
//  }
//
//  printf("v2 = %d\n", *v2);
//
//  pstIntHash->DeleteByKey(10086);
//  printf("UsedNum = %d\n", pstIntHash->GetUsedNodeNumber());
//
//  v2 = pstIntHash->GetValuePtrByKey(10086);
//
//  if(v2 == NULL)
//  {
//      printf("v2 == NULL\n");
//  }
//
//  pstIntHash->EraseAll();
//  printf("UsedNum = %d\n", pstIntHash->GetUsedNodeNumber());
//
//  v1 = pstIntHash->GetValuePtrByKey(32186690);
//
//  if(v1 == NULL)
//  {
//      printf("v1 == NULL\n");
//  }
//
//  delete [] pszBuffer;
//
//  return 0;
//}


