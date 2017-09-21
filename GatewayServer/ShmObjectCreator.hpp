#ifndef __SHM_OBJECT_CREATOR_HPP__
#define __SHM_OBJECT_CREATOR_HPP__

#include <new>
#include "SharedMemory.hpp"

template <class TYPE_ShmObject>
class CShmObjectCreator
{
public:

	static TYPE_ShmObject* CreateObject(
		CSharedMemory* pstSharedMemory,
		const char* pszKeyFileName,
		const unsigned char ucKeyPrjID)
	{
		int iRet = 0;
		pstSharedMemory->CreateShmSegment(pszKeyFileName, ucKeyPrjID, sizeof(TYPE_ShmObject));
		TYPE_ShmObject* pTYPE_ShmObject = NULL;;
		pTYPE_ShmObject = new((void*)pstSharedMemory->GetFreeMemoryAddress()) TYPE_ShmObject();
		if(!pTYPE_ShmObject)
		{
			return NULL;
		}
		iRet = pstSharedMemory->UseShmBlock(sizeof(TYPE_ShmObject));
		if(iRet < 0)
		{
			return NULL;
		}
		return pTYPE_ShmObject;
	}

	static TYPE_ShmObject* CreateObject(
		CSharedMemory* pstSharedMemory,
		const char* pszKeyFileName,
		const unsigned char ucKeyPrjID,
		const int iAddistionalSize)
	{
		int iRet = 0;
		int iSharedMemorySize = sizeof(TYPE_ShmObject) + iAddistionalSize;
        
		pstSharedMemory->CreateShmSegment(pszKeyFileName, ucKeyPrjID, iSharedMemorySize);
		TYPE_ShmObject* pTYPE_ShmObject = NULL;;
		pTYPE_ShmObject = new((void*)pstSharedMemory->GetFreeMemoryAddress()) TYPE_ShmObject(iAddistionalSize);
		if(!pTYPE_ShmObject)
		{
			return NULL;
		}
		iRet = pstSharedMemory->UseShmBlock(iSharedMemorySize);
		if(iRet < 0)
		{
			return NULL;
		}
		return pTYPE_ShmObject;
	}


};


#endif
