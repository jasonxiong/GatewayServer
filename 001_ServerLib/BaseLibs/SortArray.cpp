
#include "SortArray.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 排序数组算法, 参考自幻想代码
// 排序的顺序为按 compar 返回值进行排序. 即:
// compar(1, 2): 返回-1, 则从小到大排序
// compar(1, 2): 返回1, 则从大到小排序

// 查找一个值为key的元素
// 返回: piEqual为1时, 返回找到等于key的索引, piEqual为0时, 找到第一个小于key的值
int MyBSearch (const void *key, const void *base, int nmemb, int size, int *piEqual, int (*compar) (const void *, const void *))
{
    int l, u, idx;
    const void *p, *p2;
    int comparison, comparison2;

    *piEqual = 0;
    if (!nmemb) return 0;
    l = 0;
    u = nmemb;

    while (l < u)
    {
        idx = (l + u) / 2;
        p = (void *) (((const char *) base) + (idx * size));
        comparison = (*compar) (key, p);

        if (comparison == 0)
        {
            *piEqual = 1;
            return idx;
        }
        else if (comparison < 0)
        {
            if (idx == 0) return idx;

            p2 = (void *) (((const char *) base) + ((idx - 1) * size));
            comparison2 = (*compar) (key, p2);

            if (comparison2 > 0) return idx;

            u = idx;
        }
        else /*if (comparison > 0)*/
        {
            l = idx + 1;
        }
    }

    return u;
}

// 插入一个值为key的元素
// iUnique: 1为不允许重复, 0 - 为允许重复
// 返回值: -1 失败, >=0 成功
int MyBInsert (const void *key, const void *base, int *pnmemb, int size, int iUnique, int (*compar) (const void *, const void *))
{
    int i, iInsert, iEqu;

    iInsert = MyBSearch(key, base, *pnmemb, size, &iEqu, compar);
    if (iEqu && iUnique) return -1;
    if (iInsert < *pnmemb)
        for (i = *pnmemb; i > iInsert; i--) memcpy((char *)base + i * size, (char *)base + (i - 1)*size, size);
    memcpy((char *)base + iInsert*size, key, size);
    (*pnmemb)++;
    return iInsert;
}

// 删除索引为index的元素
// 返回值: -1 失败, 0 成功
int MyIDelete (const void *base, int *pnmemb, int size, int index)
{
    int i;

    if (0 > index || index >= *pnmemb)
        return -1;

    for (i = index; i < (*pnmemb) - 1; i++) memcpy((char *)base + i * size, (char *)base + (i + 1)*size, size);
    (*pnmemb)--;

    return 0;
}

// 删除值为key的元素
// 返回值: -1 失败, 0 成功
int MyBDelete (const void *key, const void *base, int *pnmemb, int size, int (*compar) (const void *, const void *))
{
    int i, iDelete, iEqu;

    iDelete = MyBSearch(key, base, *pnmemb, size, &iEqu, compar);
    if (!iEqu) return -1;
    if (iDelete < *pnmemb)
        for (i = iDelete; i < (*pnmemb) - 1; i++) memcpy((char *)base + i * size, (char *)base + (i + 1)*size, size);
    (*pnmemb)--;
    return 0;
}


// 插入一个值为key的元素
// 移动元素使用memmove，提高效率
// iUnique: 1为不允许重复, 0 - 为允许重复
// 返回值: -1 失败, >=0 成功
int MyBInsert2 (const void *key, const void *base, int *pnmemb, int size, int iUnique, int (*compar) (const void *, const void *))
{
    int iInsert, iEqu;

    iInsert = MyBSearch(key, base, *pnmemb, size, &iEqu, compar);

    if (iEqu && iUnique)
    {
        return -1;
    }

    if (iInsert < (*pnmemb))
    {
        memmove((char*)base + (iInsert+1)*size, (char*)base + iInsert*size, (*pnmemb-iInsert)*size);
    }

    memcpy((char*)base + iInsert* size, key, size);
    ++(*pnmemb);

    return iInsert;

}


// 删除值为key的元素
// 移动元素使用memmove，提高效率
// 返回值: -1 失败, 0 成功
int MyBDelete2 (const void *key, const void *base, int *pnmemb, int size, int (*compar) (const void *, const void *))
{
    int iDelete, iEqu;

    iDelete = MyBSearch(key, base, *pnmemb, size, &iEqu, compar);
    if (!iEqu)
    {
        return -1;
    }

    if (iDelete < *pnmemb)
    {
        memmove((char*)base + iDelete * size, (char *)base + (iDelete + 1)*size,  (*pnmemb - iDelete - 1)*size);
    }
    (*pnmemb)--;

    return 0;
}

// 查找一个值为key的元素，且要求该元素通过objcmp比较相等。该元素以key为索引值，但相同key值允许重复
// 返回: piEqual为1时, 返回找到等于key的索引, piEqual为0时, 找到第一个小于key的值
// key相同时，通过objcmp来比较是真的相同
int MyBSearchDup (const void *key, const void *base, int nmemb, int size, int *piEqual, int (*compar) (const void *, const void *), int (*objcmp)(const void *, const void *))
{
    // 先找到一个相同key， 然后再向前向后寻找相同obj
    int u = MyBSearch(key, base, nmemb, size, piEqual, compar);

    if (*piEqual == 1)
    {
        *piEqual = 0;
        for (int i = u; i >= 0; --i)
        {
            void *pobj = (void *) (((const char *) base) + (i * size));
            int keycomparison = (*compar)(key, pobj);
            if (keycomparison != 0)
            {
                break;
            }

            int objcmpariosn = (*objcmp)(key, pobj);
            if (objcmpariosn == 0)
            {
                *piEqual = 1;
                return i;
            }
        }
        for (int i = u; i < nmemb; ++i)
        {
            void *pobj = (void *) (((const char *) base) + (i * size));
            int keycomparison = (*compar)(key, pobj);
            if (keycomparison != 0)
            {
                break;
            }

            int objcmpariosn = (*objcmp)(key, pobj);
            if (objcmpariosn == 0)
            {
                *piEqual = 1;
                return i;
            }
        }
    }

    return u;
}

// 插入一个值为key的元素，且要求该元素通过objcmp比较相等。该元素以key为索引值，但相同key值允许重复
// 移动元素使用memmove，提高效率
// iUnique: 1为不允许重复, 0 - 为允许重复
// bTail: true为加到相同key最后， false为加到最前
// 返回值: -1 失败, >=0 成功
int MyBInsertDup (const void *key, const void *base, int *pnmemb, int size, int iUnique, bool bTail, int (*compar) (const void *, const void *),  int (*objcmp)(const void *, const void *))
{
    int iInsert, iEqu;

    iInsert = MyBSearch(key, base, *pnmemb, size, &iEqu, compar);

    if (iEqu)
    {
        while(iInsert >= 0 && iInsert < *pnmemb)
        {
            void *pobj = (void *) (((const char *) base) + (iInsert * size));
            int keycomparison = (*compar)(key, pobj);
            if (keycomparison != 0)
            {
                break;
            }
            if (iUnique)
            {
                int objcmpariosn = (*objcmp)(key, pobj);
                if (objcmpariosn == 0)
                {
                    return -1;
                }
            }
            bTail ? ++iInsert : --iInsert;
        }
    }

    if (iInsert < 0 || iInsert > *pnmemb)
    {
        return -1;
    }

    if (iInsert < (*pnmemb))
    {
        memmove((char*)base + (iInsert+1)*size, (char*)base + iInsert*size, (*pnmemb-iInsert)*size);
    }

    memcpy((char*)base + iInsert* size, key, size);
    ++(*pnmemb);

    return iInsert;

}


// 删除值为key的元素
// 移动元素使用memmove，提高效率
// 返回值: -1 失败, 0 成功
int MyBDeleteDup (const void *key, const void *base, int *pnmemb, int size, int (*compar) (const void *, const void *), int (*objcmp)(const void *, const void *))
{
    int iDelete, iEqu;

    iDelete = MyBSearchDup(key, base, *pnmemb, size, &iEqu, compar, objcmp);
    if (!iEqu)
    {
        return -1;
    }


    if (iDelete < *pnmemb)
    {
        memmove((char*)base + iDelete * size, (char *)base + (iDelete + 1)*size,  (*pnmemb - iDelete - 1)*size);
    }
    (*pnmemb)--;

    return 0;
}

// 返回删除指定成员值后剩下的数组（无序）元素个数
int MyDeleteArrayElement (const void *key, const void *base, int *pnmemb, int size, int (*compar) (const void *, const void *))
{
    int iIdx = 0;
    for (int i=0; i<*pnmemb; ++i)
    {
        void *p = (void *) (((const char *) base) + (i * size));
        if ((*compar)(p, key) != 0)
        {
            if (i != iIdx)
            {
                memcpy((char *)base + iIdx * size, (char *)base + i * size, size);
            }
            iIdx++;
        }
    }

    return iIdx;
}

int MyDeleteArray_If(const void *base, int *pnmemb, int iSize, bool (*IfPred)(const void *))
{
    int iRealIdx = 0;
    for (int i = 0; i < *pnmemb; i++)
    {
        void *p = (void *) (((const char *) base) + (i * iSize));
        if ((*IfPred)(p))
        {
            if (i != iRealIdx)
            {
                memcpy((char *)base + iRealIdx * iSize, (char *)base + i * iSize, iSize);
            }
            iRealIdx++;
        }

    }

    *pnmemb = iRealIdx;
    return iRealIdx;
}
