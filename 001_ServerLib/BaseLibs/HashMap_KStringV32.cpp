
#include "HashMap_KStringV32.hpp"

#include "LogAdapter.hpp"


using namespace ServerLib;



CHashMap_KStringV32* CHashMap_KStringV32::CreateHashMap(char* pszMemoryAddress,
        const unsigned int uiFreeMemorySize,
        const int iNodeNumber)
{
    if(CountSize(iNodeNumber) > uiFreeMemorySize)
    {
        return NULL;
    }

    return new(pszMemoryAddress) CHashMap_KStringV32(iNodeNumber);
}


int CHashMap_KStringV32::AttachHashMap(char* pszMemoryAddress)
{
    __ASSERT_AND_LOG(m_iNodeNumber >= 0);
    m_pastHashNodeAssist = (THashMapNode_KSTRINGV32*)((char*)(pszMemoryAddress) + sizeof(CHashMap_KStringV32));
    m_paiHashFirstIndexAssist = (int*)((char*)(m_pastHashNodeAssist) + sizeof(THashMapNode_KSTRINGV32) * m_iNodeNumber);
    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNodeAssist,(unsigned int)m_paiHashFirstIndexAssist);

    m_pastHashNode = m_pastHashNodeAssist;
    m_paiHashFirstIndex = m_paiHashFirstIndexAssist;

    return 0;
}

////设置m_iUseFlagNW
//
//int CHashMap_KStringV32::SetUseFlagNW(const int iUseFlag)
//{
//	m_iUseFlagNW = iUseFlag;
//	return 0;
//}

////获取m_iUseFlagNW
//
//int& CHashMap_KStringV32::GetUseFlagNW()
//{
//	return m_iUseFlagNW;
//}

////设置m_iUseFlagNR
//
//int CHashMap_KStringV32::SetUseFlagNR(const int iUseFlag)
//{
//	m_iUseFlagNR = iUseFlag;
//	return 0;
//}

////获取m_iUseFlagNR
//
//int& CHashMap_KStringV32::GetUseFlagNR()
//{
//	return m_iUseFlagNR;
//}


size_t CHashMap_KStringV32::CountSize(const int iNodeNumber)
{
    return sizeof(CHashMap_KStringV32) + (sizeof(THashMapNode_KSTRINGV32) + sizeof(int)) * iNodeNumber;
}


CHashMap_KStringV32::CHashMap_KStringV32()
{


}


CHashMap_KStringV32::CHashMap_KStringV32(int iNodeNumber)
{
    __ASSERT_AND_LOG(iNodeNumber > 0);

    //m_iUseFlagNR=EHNS_IDLE;// 把HashMap的状态设为idle状态
    //m_iUseFlagNW=EHNS_IDLE;// 把HashMap的状态设为idle状态

    m_iErrorNO = 0;

    m_iNodeNumber = iNodeNumber;

    m_pastHashNode = (THashMapNode_KSTRINGV32*)((char*)(this) + sizeof(CHashMap_KStringV32));
    m_paiHashFirstIndex = (int*)((char*)(m_pastHashNode) + sizeof(THashMapNode_KSTRINGV32) * iNodeNumber);
    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNode,(unsigned int)m_paiHashFirstIndex);

    EraseAll();
}


CHashMap_KStringV32::~CHashMap_KStringV32()
{
}


int CHashMap_KStringV32::EraseAll()
{
    m_iUsedNodeNumber = 0;
    m_iFirstFreeIndex = 0;

    int i;
    for(i = 0; i < m_iNodeNumber; ++i)
    {
        m_pastHashNode[i].m_iHashNext = i + 1;
        m_pastHashNode[i].m_iIsNodeUsed = EHNS_KSTRINGV32_FREE;
    }

    m_pastHashNode[m_iNodeNumber-1].m_iHashNext = -1;

    for(i = 0; i < m_iNodeNumber; ++i)
    {
        m_paiHashFirstIndex[i] = -1;
    }

    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNode,(unsigned int)m_paiHashFirstIndex);
    return 0;
}


int CHashMap_KStringV32::GetValueByIndex(const int iNodeIndex, int& riValue)
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

    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNode,(unsigned int)m_paiHashFirstIndex);
    return 0;
}


//!指定索引值来获取Hash节点，一般用于遍历中，返回值0表示成功

int CHashMap_KStringV32::GetKeyByIndex(const int iNodeIndex, TDataStringV32& riHashMapKey)
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

    memcpy(&riHashMapKey,&(m_pastHashNode[iNodeIndex].m_stPriKey),sizeof(riHashMapKey) );

    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNode,(unsigned int)m_paiHashFirstIndex);

    return 0;
}

//////////////用于Assist工具////////////////////////


int CHashMap_KStringV32::GetValueByIndexAssist(const int iNodeIndex, int& riValue)
{
    if(iNodeIndex < 0 || iNodeIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    if(!m_pastHashNodeAssist[iNodeIndex].m_iIsNodeUsed)
    {
        return -2;
    }

    riValue = m_pastHashNodeAssist[iNodeIndex].m_iValue;

    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNodeAssist,(unsigned int)m_paiHashFirstIndexAssist);
    return 0;
}


//!指定索引值来获取Hash节点，一般用于遍历中，返回值0表示成功

int CHashMap_KStringV32::GetKeyByIndexAssist(const int iNodeIndex, TDataStringV32& riHashMapKey)
{
    if(iNodeIndex < 0 || iNodeIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    if(!m_pastHashNodeAssist[iNodeIndex].m_iIsNodeUsed)
    {
        return -2;
    }

    memcpy(&riHashMapKey,&(m_pastHashNodeAssist[iNodeIndex].m_stPriKey),sizeof(riHashMapKey) );

    //TRACESVR("pastHashNode size:%u, HashFirstIndex size:%u \n",(unsigned int)m_pastHashNodeAssist,(unsigned int)m_paiHashFirstIndexAssist);

    return 0;
}









///////////////////////////////////////





int CHashMap_KStringV32::GetValueByKey(const TDataStringV32 stPriKey, int& riValue)
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


int* CHashMap_KStringV32::GetValuePtrByKey(const TDataStringV32 stPriKey)
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


int CHashMap_KStringV32::DeleteByKey(const TDataStringV32 stPriKey, int& riValue)
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

        m_pastHashNode[iCurrentIndex].m_iIsNodeUsed = EHNS_KSTRINGV32_FREE;
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


int CHashMap_KStringV32::InsertValueByKey(const TDataStringV32 stPriKey, const int iValue)
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
    m_pastHashNode[iNowAssignIdx].m_iIsNodeUsed = EHNS_KSTRINGV32_USED;
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


int CHashMap_KStringV32::UpdateValueByKey(const TDataStringV32 stPriKey, const int iValue)
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


int CHashMap_KStringV32::ReplaceValueByKey(const TDataStringV32 stPriKey, const int iValue)
{
    int iHashIndex = HashKeyToIndex(stPriKey);

    if(iHashIndex < 0 || iHashIndex >= m_iNodeNumber)
    {
        SetErrorNO(EEN_HASH_MAP__INVALID_INDEX);

        return -1;
    }

    int iPreIndex = -1;
    /*assert(m_paiHashFirstIndex);
    assert(m_pastHashNode);*/
    int iCurrentIndex = m_paiHashFirstIndex[iHashIndex];

    //while(iCurrentIndex != -1)
    while(iCurrentIndex >= 0)
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
    m_pastHashNode[iNowAssignIdx].m_iIsNodeUsed = EHNS_KSTRINGV32_USED;
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


const unsigned int SEED = 131;
const unsigned int MASK = 0x7FFFFFFF;

// BKDR Hash Function

int CHashMap_KStringV32::BKDRHash(const char* szStr,int iStrLength) const
{
    //unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    unsigned int *piTemp = (unsigned int*)szStr;

    //将szStr按每个字节进行操作
    unsigned int i;
    for( i = 0; i < iStrLength / sizeof(int); ++i)
    {
        //hash += piTemp[i];
        hash = hash * SEED + (piTemp[i]);
    }

    if(iStrLength % sizeof(int) > 0)
    {
        unsigned char* pByte = (unsigned char*)szStr;
        pByte += (iStrLength - (iStrLength % sizeof(int)));
        unsigned int uiTemp = 0; // 之前未初始化，导致对相同的key生成的hash location不相同
        memcpy((void *)&uiTemp, (const void *)pByte, iStrLength%sizeof(int));
        hash += uiTemp;
    }

    //int i=0;
    //for(i=0;i<iLength;i++)
    //{
    //	//hash = hash * seed + (*szStr++);
    //	hash = hash * SEED + (*szStr++);
    //}

    //return (hash & 0x7FFFFFFF);
    return (hash & MASK);
}



int CHashMap_KStringV32::HashKeyToIndex(const TDataStringV32 stPriKey) const
{
    //int hash_index=BKDRHash(stPriKey.KeyStringV32,strlen(stPriKey.KeyStringV32));
    int iRealLength = strnlen(stPriKey.KeyStringV32,sizeof(stPriKey.KeyStringV32));
    int hash_index=BKDRHash(stPriKey.KeyStringV32,iRealLength);

    hash_index=hash_index % m_iNodeNumber;
    //TRACESVR("HashKeyToIndex.Key=%s, hash_index=%d ,m_iNodeNumber=%d  \n",(char*)stPriKey.KeyStringV32, hash_index,m_iNodeNumber);
    return hash_index;
    //size_t uiKeyLength = sizeof(stPriKey);
    //unsigned int uiHashSum = 0;
    //unsigned int *piTemp = (unsigned int*)&stPriKey;

    ////目前Hash算法实现比较简单只是将Key值的每个字节的值加起来并对SIZE取模
    //unsigned int i;
    //for( i = 0; i < uiKeyLength / sizeof(unsigned int); ++i)
    //{
    //	uiHashSum += piTemp[i];
    //}

    //if(uiKeyLength % sizeof(unsigned int) > 0)
    //{
    //	unsigned char* pByte = (unsigned char*)&stPriKey;
    //	pByte += (uiKeyLength - (uiKeyLength % sizeof(unsigned int)));
    //	unsigned int uiTemp;
    //	memcpy((void *)&uiTemp, (const void *)pByte, uiKeyLength%sizeof(unsigned int));
    //	uiHashSum += uiTemp;
    //}

    //uiHashSum = (uiHashSum & ((unsigned int)0x7fffffff));

    //int hash_index = (int)(uiHashSum % m_iNodeNumber);

    //return hash_index;
}


bool CHashMap_KStringV32::CompareKey(const TDataStringV32& key1,const TDataStringV32& key2)
{
    //if(memcmp((const void*)&key1, (const void*)&key2, sizeof(TDataStringV32)) == 0)
    //if(strcmp((const char*)key1.KeyStringV32,(const char*)key2.KeyStringV32) == 0)
    if(strncmp((const char*)key1.KeyStringV32,(const char*)key2.KeyStringV32,sizeof(key1.KeyStringV32)) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//
//#include <stdio.h>
//
//int main()
//{
//	printf("CountSize %d \n", CHashMap_KStringV32::CountSize(0));
//	printf("CountSize %d \n", CHashMap_KStringV32::CountSize(1));
//	printf("CountSize %d \n", CHashMap_KStringV32::CountSize(2));
//	printf("CountSize %d \n", CHashMap_KStringV32::CountSize(10));
//	printf("CountSize %d \n", CHashMap_KStringV32::CountSize(100));
//	printf("CountSize %d \n", CHashMap_KStringV32::CountSize(100000));
//
//	char* pszBuffer = new char[2500000];
//	CHashMap_KStringV32* pstIntHash = new(pszBuffer) CHashMap_KSTRINGV32(100000);
//
//	pstIntHash->InsertValueByKey(32186690, 10000);
//	pstIntHash->ReplaceValueByKey(10086, 10);
//	pstIntHash->ReplaceValueByKey(32186690, 20000);
//
//	printf("UsedNum = %d\n", pstIntHash->GetUsedNodeNumber());
//
//	int* v1 = pstIntHash->GetValuePtrByKey(32186690);
//
//	if(v1 == NULL)
//	{
//		printf("v1 == NULL\n");
//
//		return -1;
//	}
//
//	printf("v1 = %d\n", *v1);
//
//	pstIntHash->UpdateValueByKey(32186690, 10000);
//
//	v1 = pstIntHash->GetValuePtrByKey(32186690);
//
//	if(v1 == NULL)
//	{
//		printf("v1 == NULL\n");
//
//		return -1;
//	}
//
//	printf("v1 = %d\n", *v1);
//
//	int* v2 = pstIntHash->GetValuePtrByKey(10086);
//
//	if(v2 == NULL)
//	{
//		printf("v2 == NULL\n");
//
//		return -1;
//	}
//
//	printf("v2 = %d\n", *v2);
//
//	pstIntHash->DeleteByKey(10086);
//	printf("UsedNum = %d\n", pstIntHash->GetUsedNodeNumber());
//
//	v2 = pstIntHash->GetValuePtrByKey(10086);
//
//	if(v2 == NULL)
//	{
//		printf("v2 == NULL\n");
//	}
//
//	pstIntHash->EraseAll();
//	printf("UsedNum = %d\n", pstIntHash->GetUsedNodeNumber());
//
//	v1 = pstIntHash->GetValuePtrByKey(32186690);
//
//	if(v1 == NULL)
//	{
//		printf("v1 == NULL\n");
//	}
//
//	delete [] pszBuffer;
//
//	return 0;
//}
