
#ifndef __INTEGER_HASH_HPP__
#define __INTEGER_HASH_HPP__

#include <stdio.h>
#include <string.h>



template<int iSize, class InfoType>
class CHashNodeMng
{
public:
	CHashNodeMng();
	~CHashNodeMng();
	int Initialize();

	int GetHashKey(int iIdx, int &riKey);
	int SetHashKey(int iIdx, int iKey);
	int GetNextIdx(int iCurIdx, int &riNextIdx);
	int SetNextIdx(int iCurIdx, int iNextIdx);
	int GetNodeInfo(int iIdx, InfoType *rpInfo);
	int SetNodeInfo(int iIdx, InfoType *pInfo);
	int GetFreeNodeIdx(int &riFreeIdx);
	int GetUsedNodeNum();

	InfoType& GetNodeByIdx(int iIdx);

private:
	struct TNode
	{
		InfoType m_stNodeInfo;
		int m_iNextIdx;
	}m_astNode[iSize];
	int m_iFreeHead;

};

template<int iSize, class InfoType>
CHashNodeMng<iSize, InfoType>::CHashNodeMng()
{
	Initialize();
}

template<int iSize, class InfoType>
CHashNodeMng<iSize, InfoType>::~CHashNodeMng()
{
}
template<int iSize, class InfoType>
int CHashNodeMng<iSize, InfoType>::Initialize()
{
	m_iFreeHead = 0;
	memset(&m_astNode, -1, sizeof(m_astNode));
	return 0;
}
template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::GetHashKey(int iIdx, int &riKey)
{
	if(iIdx < 0 || iIdx >= m_iFreeHead)
	{
		return -1;
	}
	riKey = m_astNode[iIdx].m_stNodeInfo.GetHashKey();
	return 0;
}
template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::SetHashKey(int iIdx, int iKey)
{
	if(iIdx < 0 || iIdx >= m_iFreeHead)
	{
		return -1;
	}
	m_astNode[iIdx].m_stNodeInfo.SetHashKey(iKey);
	return 0;
}

template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::GetNextIdx(int iCurIdx, int &riNextIdx)
{
	if(iCurIdx < 0 || iCurIdx >= m_iFreeHead)
	{
		return -1;
	}
	riNextIdx = m_astNode[iCurIdx].m_iNextIdx;
	return 0;
}
template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::SetNextIdx(int iCurIdx, int iNextIdx)
{
	if(iCurIdx < 0 || iCurIdx >= m_iFreeHead ||iNextIdx < -1 ||  iNextIdx >= m_iFreeHead)
	{
		return -1;
	}
	m_astNode[iCurIdx].m_iNextIdx = iNextIdx;
	return 0;
}

template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::GetNodeInfo(int iIdx, InfoType *rpInfo)
{
	if(iIdx < 0 || iIdx >= m_iFreeHead || !rpInfo)
	{
		return -1;
	}
	memcpy(rpInfo, &m_astNode[iIdx].m_stNodeInfo, sizeof(m_astNode[iIdx].m_stNodeInfo));
	return 0;
}
template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::SetNodeInfo(int iIdx, InfoType *pInfo)
{
	if(iIdx < 0 || iIdx >= m_iFreeHead || !pInfo)
	{
		return -1;
	}
	memcpy(&m_astNode[iIdx].m_stNodeInfo, pInfo, sizeof(m_astNode[iIdx].m_stNodeInfo));
	return 0;
}
template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::GetFreeNodeIdx(int &riFreeIdx)
{
	if(m_iFreeHead >= iSize)
	{
		riFreeIdx = -1;
		return -1;
	}
	riFreeIdx = m_iFreeHead++;
	return 0;
}

template<int iSize, class InfoType>
inline int CHashNodeMng<iSize, InfoType>::GetUsedNodeNum()
{
	return m_iFreeHead;
}

template<int iSize, class InfoType>
inline InfoType& CHashNodeMng<iSize, InfoType>::GetNodeByIdx(int iIdx)
{
	return m_astNode[iIdx].m_stNodeInfo;
}



//////////////////////////////////////////////////////////////////////////



template<int iSize, class InfoType>
class CIntegerHash
{
public:
	CIntegerHash();//初始化
	~CIntegerHash();//空
	int Initialize();
	int Add(int iKey, InfoType *pNodeInfo);
	int Get(int iKey, InfoType *pNodeInfo);
	int GetNodeNum();
	int GetAllNodeKey(int &riAllKeyNum, int *piAllKey);
	InfoType& GetNodeByIdx(int iIdx);
	//InfoType& GetNodeByKey(int iKey);
	int Show(FILE *fpOut);
private:
	int Key2Idx(int iKey);
	CHashNodeMng<iSize, InfoType> m_stNodeMng;
	int m_aiSlot[iSize];
};

template<int iSize, class InfoType>
CIntegerHash<iSize, InfoType>::CIntegerHash()
{
	Initialize();
}
template<int iSize, class InfoType>
CIntegerHash<iSize, InfoType>::~CIntegerHash()
{

}

template<int iSize, class InfoType>
int CIntegerHash<iSize, InfoType>::Initialize()
{
	m_stNodeMng.Initialize();
	memset(&m_aiSlot, -1, sizeof(m_aiSlot));
	return 0;
}

template<int iSize, class InfoType>
int CIntegerHash<iSize, InfoType>::GetNodeNum()
{
	return m_stNodeMng.GetUsedNodeNum();
}

template<int iSize, class InfoType>
int CIntegerHash<iSize, InfoType>::GetAllNodeKey(int &riAllKeyNum, int *piAllKey)
{
	if(riAllKeyNum <=0 || !piAllKey)
	{
		return -1;
	}
	if(riAllKeyNum > GetNodeNum())
	{
		riAllKeyNum = GetNodeNum();
	}

	int i;
	for(i = 0; i < riAllKeyNum; ++i)
	{
		m_stNodeMng.GetHashKey(i, piAllKey[i]);
	}
	return 0;
}

template<int iSize, class InfoType>
int CIntegerHash<iSize, InfoType>::Add(int iKey, InfoType *pNodeInfo)
{
	if(!pNodeInfo)
	{
		return -1;
	}

	int iIdx;
	int iNextIdx;
	int iSlotIdx;
	int iTmpKey;

	iSlotIdx = Key2Idx(iKey);
	//collision
	if(m_aiSlot[iSlotIdx] >= 0)
	{
		iNextIdx = m_aiSlot[iSlotIdx]; 
		do
		{
			iIdx = iNextIdx;
			m_stNodeMng.GetHashKey(iIdx, iTmpKey);
			if(iKey == iTmpKey)
			{
				//重复，覆盖掉以前的NodeInfo
				m_stNodeMng.SetNodeInfo(iIdx, pNodeInfo);
				return 0;
			}

			m_stNodeMng.GetNextIdx(iIdx, iNextIdx);
		}while(-1 != iNextIdx);
	}//end ifm_aiSlot[iSlotIdx] >= 0

	//未找到重复的Key
	m_stNodeMng.GetFreeNodeIdx(iNextIdx);
	if(iNextIdx < 0)
	{
		//NodeMng管理的节点满了
		return -2;
	}

	m_stNodeMng.SetNodeInfo(iNextIdx, pNodeInfo);
	m_stNodeMng.SetNextIdx(iNextIdx, m_aiSlot[iSlotIdx]);
	m_aiSlot[iSlotIdx] = iNextIdx;
	return 0;
}

template<int iSize, class InfoType>
int CIntegerHash<iSize, InfoType>::Get(int iKey, InfoType *pNodeInfo)
{
	if(!pNodeInfo)
	{
		return -1;
	}

	int iIdx;
	int iNextIdx;
	iIdx = Key2Idx(iKey);

	if(m_aiSlot[iIdx] >= 0)
	{
		iNextIdx = m_aiSlot[iIdx];
		do
		{
			iIdx = iNextIdx;
			m_stNodeMng.GetNodeInfo(iIdx, pNodeInfo);
			if((unsigned int)iKey == pNodeInfo->GetHashKey())
			{
				return 0;
			}

			m_stNodeMng.GetNextIdx(iIdx, iNextIdx);
		}while(-1 !=iNextIdx);
	}//end if(m_aiSlot[iIdx] > 0)

	return -1;
}

template<int iSize, class InfoType>
inline InfoType& CIntegerHash<iSize, InfoType>::GetNodeByIdx(int iIdx)
{
	return m_stNodeMng.GetNodeByIdx(iIdx);
}

template<int iSize, class InfoType>
inline int CIntegerHash<iSize, InfoType>::Key2Idx(int iKey)
{
	return (iKey * 2654435769u) % iSize ;
}

template<int iSize, class InfoType>
int CIntegerHash<iSize, InfoType>::Show(FILE *fpOut)
{
	int iSlotIdx = 0;
	int iIdx = 0;
	int iNextIdx = 0;
	int i = 0;
	int iNodeNumOfSlot = 0;
	int iCollisionNum = 0;
	InfoType stNodeInfo;

	if(!fpOut)
	{
		return -1;
	}

	fprintf(fpOut, "\nSHOW HASH TABLE:\n");

	for(iSlotIdx = 0; iSlotIdx < iSize; ++iSlotIdx)
	{
		if(m_aiSlot[iSlotIdx] < 0)
		{
			continue;
		}

		fprintf(fpOut, "HASH[%06d]->", iSlotIdx);

		iNodeNumOfSlot = 0;
		iNextIdx = m_aiSlot[iSlotIdx];
		do{
			++iNodeNumOfSlot;
			if(iNodeNumOfSlot > 1)
			{
				++iCollisionNum;
			}
			iIdx = iNextIdx;
			m_stNodeMng.GetNodeInfo(iIdx, &stNodeInfo);
			stNodeInfo.Show(fpOut);
			m_stNodeMng.GetNextIdx(iIdx, iNextIdx);
		} while(-1 != iNextIdx);

		fprintf(fpOut, "NodeNum of This Slot %d. END\n", iNodeNumOfSlot);
	}

	fprintf(fpOut, " TotalNodeNum %d, Collision Num %d.\n", GetNodeNum(), iCollisionNum);

	fflush(fpOut);

	return 0;

}



#endif 

