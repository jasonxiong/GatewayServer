
#include "UinPrivManager.hpp"

static int CompareUinPrivInfo(const void* pst1, const void* pst2)
{
    TUinPrivInfo* pstData1 = (TUinPrivInfo*)pst1;
    TUinPrivInfo* pstData2 = (TUinPrivInfo*)pst2;

    if (pstData1->m_uiUin == pstData2->m_uiUin)
    {
        return 0;
    }
    else
    {
        if (pstData1->m_uiUin < pstData2->m_uiUin)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}

size_t CUinPrivManager::GetObjTotalSize()
{
    if (m_iObjTotalSize > 0)
    {
        return m_iObjTotalSize;
    }

    TRACESVR("********************** OBJECT MEMORY STATICS *********************\n");

    // UinPrivInfo对象
    size_t iUinPrivInfoSize = sizeof(TUinPrivInfo) * m_iMaxUinNumber;
    m_iObjTotalSize += iUinPrivInfoSize;
    TRACESVR("UinPrivInfo: Number = %d, UnitSize = %lu, Total = %lu\n",
             m_iMaxUinNumber, (unsigned long)sizeof(TUinPrivInfo), (unsigned long)iUinPrivInfoSize);

    // 两块共享内存
    m_iObjTotalSize = m_iObjTotalSize * 2;

    // 主备控制器标识
    m_iObjTotalSize += sizeof(int*);

    // 总计
    TRACESVR("Total Memory: %luB, %luMB\n", (unsigned long)m_iObjTotalSize, (unsigned long)m_iObjTotalSize/1024/1024);
    TRACESVR("*****************************************************************\n");

    return m_iObjTotalSize;
}

int CUinPrivManager::Initialize(int iUinNumber,const char* pszShmName, bool bIsUinPrivServer)
{
    m_iObjTotalSize = 0;
    m_iMaxUinNumber = iUinNumber;
    size_t iSharedMmorySize = GetObjTotalSize();

    // 创建共享内存
    int iRet = m_stSharedMemory.CreateShmSegment(pszShmName, 'o', iSharedMmorySize);
    if(iRet < 0)
    {
        return -1;
    }

    m_piActiveController = (int*)m_stSharedMemory.GetFreeMemoryAddress();
    m_stSharedMemory.UseShmBlock(sizeof(int));

    m_astUinPrivControllers[0].m_piNumber = (int*)m_stSharedMemory.GetFreeMemoryAddress();
    m_stSharedMemory.UseShmBlock(sizeof(int));

    m_astUinPrivControllers[0].m_pstCache = (TUinPrivInfo*)m_stSharedMemory.GetFreeMemoryAddress();
    m_stSharedMemory.UseShmBlock(sizeof(TUinPrivInfo) * m_iMaxUinNumber);

    m_astUinPrivControllers[1].m_piNumber = (int*)m_stSharedMemory.GetFreeMemoryAddress();
    m_stSharedMemory.UseShmBlock(sizeof(int));

    m_astUinPrivControllers[1].m_pstCache = (TUinPrivInfo*)m_stSharedMemory.GetFreeMemoryAddress();
    m_stSharedMemory.UseShmBlock(sizeof(TUinPrivInfo) * m_iMaxUinNumber);

    // 初始化共享内存
    if (bIsUinPrivServer)
    {
        *m_piActiveController = 0;
        *m_astUinPrivControllers[0].m_piNumber = 0;
        *m_astUinPrivControllers[1].m_piNumber = 0;
    }

    TRACESVR("Obj shared memory: Total allocated %lu, Used %lu, Free %lu\n",
             (unsigned long)iSharedMmorySize,
             (unsigned long)GetObjTotalSize(),
             (unsigned long)m_stSharedMemory.GetFreeMemorySize());

    Trace();
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// 以下接口给应用程序使用

// 查询指定Uin的权限信息
//   参数: rstUinPrivInfo.m_uiUin 查询Uin
//         rstUinPrivInfo.m_uiPriv 返回权限值
//   返回值: 0 - 成功, 负值失败
TUinPrivInfo* CUinPrivManager::GetUinPrivInfo(unsigned int uiUin, bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    TUinPrivInfo stUinPrivInfo;
    stUinPrivInfo.m_uiUin = uiUin;
    return (TUinPrivInfo*)bsearch(
               &stUinPrivInfo,
               pstController->m_pstCache,
               *pstController->m_piNumber,
               sizeof(TUinPrivInfo),
               CompareUinPrivInfo);
}


// 打印
void CUinPrivManager::Trace()
{
    TRACESVR("Active is %d.\n", *m_piActiveController);

    int i = 0;
    for (i = 0; i < 2; i++)
    {
        int k = 0;
        for (k = 0; k < *m_astUinPrivControllers[i].m_piNumber; k++)
        {
            //TRACESVR("%d:%d = %d.\n", i, k, m_astUinPrivControllers[i].m_pstCache[k].m_uiUin);
        }
    }

    return;
}

//////////////////////////////////////////////////////////////////////////
// 以下接口给ServerUinPrivServer使用

// 交换ActiveController
void CUinPrivManager::SwitchActiveCache()
{
    *m_piActiveController = (*m_piActiveController + 1) % 2;
}

// 清空备份缓冲区
int CUinPrivManager::CleanBackupCache()
{
    TUinPrivController* pstBackupController = &m_astUinPrivControllers[1 - *m_piActiveController];
    *pstBackupController->m_piNumber = 0;
    return 0;
}

int CUinPrivManager::InsertUinPrivInfo(const TUinPrivInfo& rstUinPrivInfo, bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    if (*pstController->m_piNumber >= m_iMaxUinNumber)
    {
        return -1;
    }

    // 如果权限为空, 则删除权限信息
    if (rstUinPrivInfo.m_uiPriv == 0)
    {
        return DeleteUinPrivInfo(rstUinPrivInfo.m_uiUin, bActiveCache);
    }

    // 更新已有的权限信息
    TUinPrivInfo* pstUinPrivInfo = GetUinPrivInfo(rstUinPrivInfo.m_uiUin, bActiveCache);
    if (pstUinPrivInfo)
    {
        pstUinPrivInfo->m_uiPriv = rstUinPrivInfo.m_uiPriv;
        return 0;
    }

    // 添加新的权限信息
    pstController->m_pstCache[*pstController->m_piNumber] = rstUinPrivInfo;
    (*pstController->m_piNumber)++;

    qsort(pstController->m_pstCache, *pstController->m_piNumber, sizeof(TUinPrivInfo), CompareUinPrivInfo);

    return 0;
}


int CUinPrivManager::InsertUinPrivInfoWithoutSort(const TUinPrivInfo& rstUinPrivInfo, bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    if (*pstController->m_piNumber >= m_iMaxUinNumber)
    {
        return -1;
    }

    // 添加新的权限信息
    pstController->m_pstCache[*pstController->m_piNumber] = rstUinPrivInfo;
    (*pstController->m_piNumber)++;

    return 0;
}

// 排序对象
void CUinPrivManager::SortUinPrivInfo(bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    qsort(pstController->m_pstCache, *pstController->m_piNumber, sizeof(TUinPrivInfo), CompareUinPrivInfo);
}

//删除对象还必须删除Hash
int CUinPrivManager::DeleteUinPrivInfo(const unsigned int uiUin, bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    TUinPrivInfo* pstUinPrivInfo = GetUinPrivInfo(uiUin, bActiveCache);
    if (!pstUinPrivInfo)
    {
        return 0;
    }

    int iStart = pstUinPrivInfo - pstController->m_pstCache;
    for (int i = iStart; i < m_iMaxUinNumber - 1; i++)
    {
        pstController->m_pstCache[i] = pstController->m_pstCache[i+1];
    }

    (*pstController->m_piNumber)--;

    return 0;
}


// 获取数量
int CUinPrivManager::GetUinPrivNumber(bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    return *(pstController->m_piNumber);
}

// 获取权限
const TUinPrivInfo* CUinPrivManager::GetUinPrivInfoByIndex(int iIndex, bool bActiveCache)
{
    TUinPrivController* pstController = NULL;;

    if (bActiveCache)
    {
        pstController = &m_astUinPrivControllers[*m_piActiveController];
    }
    else
    {
        pstController = &m_astUinPrivControllers[1 - *m_piActiveController];
    }

    if (iIndex < 0 || iIndex >= *(pstController->m_piNumber))
    {
        return NULL;
    }

    return &(pstController->m_pstCache[iIndex]);
}
