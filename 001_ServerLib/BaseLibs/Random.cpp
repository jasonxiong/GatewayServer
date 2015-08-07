
#include "Random.hpp"
#include "LogAdapter.hpp"

#include "TimeValue.hpp"
#include "SharedMemory.hpp"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>


using namespace ServerLib;

const int MAX_RANDOM_NUM = 1000000;

#ifdef GET_RAND_FROM_SHM
int CRandomCalculator::m_iRandRingBuffIdx = 0;
int *CRandomCalculator::m_piRandRingBuff = NULL;
#endif

int CRandomCalculator::Initialize()
{
    srand(time(NULL));
    srandom(time(NULL));

#ifdef GET_RAND_FROM_SHM
    m_piRandRingBuff = NULL;
    m_iRandRingBuffIdx = 0;

    CSharedMemory stShm;
    int iRet = stShm.CreateShmSegmentByKey(0x13579, sizeof(int) * MAX_RANDOM_NUM);
    if (iRet)
    {
        return 0;
    }

    m_piRandRingBuff = (int *)(stShm.GetFreeMemoryAddress());
    if (m_piRandRingBuff == NULL)
    {
        return 0;
    }

    for (int i = 0; i < MAX_RANDOM_NUM; i++)
    {
        m_piRandRingBuff[i] = random();
    }
#endif

    return 0;
}

int CRandomCalculator::GetMyRand()
{
#ifdef GET_RAND_FROM_SHM
    if (m_piRandRingBuff != NULL)
    {
        int iRand =  m_piRandRingBuff[m_iRandRingBuffIdx];
        m_iRandRingBuffIdx = (m_iRandRingBuffIdx + 1) % MAX_RANDOM_NUM;
        return iRand;
    }
#endif

    return random();
}

int CRandomCalculator::GetRandomNumberInRange(int iRange)
{
	if(iRange == 0)
	{
		return 0;
	}

	return GetMyRand()%iRange;
}

int CRandomCalculator::GetRandomNumber(int iMin, int iMax)
{
    int i = GetMyRand();
    if (iMin > iMax)
    {
        iMin = iMax;
    }

    return (i % (iMax - iMin + 1)) + iMin;
}

// 随机返回0~MAX_DROP_RATE之间的一个数
int CRandomCalculator::GetRandomInRangeHundredMillion()
{
    return GetRandomNumber(0, MAX_DROP_RATE);
}

//pluto jasonxiong 新增加返回0 ~ 1000 范围内的随机数
int CRandomCalculator::GetRandomInRangeThousand()
{
    return GetRandomNumber(0, 1000);
}

//jasonxiong 新增加返回0 ~ 1000 范围内的随机数
int CRandomCalculator::GetRandomInRangeTenThousand()
{
	return GetRandomNumber(0, 10000);
}

bool CRandomCalculator::GetRandomBool()
{
    int i = 0;

    i = GetMyRand();

    if (0 == (i % 2) )
    {
        return false;
    }

    return true;
}

// 是否在随机范围内
bool CRandomCalculator::IsInRangeTenThousand(int iIn)
{
    if (GetRandomNumber(1, 10000) <= iIn)
    {
        return true;
    }

    return false;
}

bool CRandomCalculator::IsInRangeHundredMillion(int iIn)
{
    if (GetRandomNumber(1, MAX_DROP_RATE) <= iIn)
    {
        return true;
    }

    return false;
}

bool CRandomCalculator::IsInGivenRange(int iRange, int iIn)
{
    if (iRange <= 0 || iIn <= 0)
    {
        return false;
    }

    if (GetRandomNumber(1, iRange) <= iIn)
    {
        return true;
    }

    return false;
}

// 生成[iLow, iHigh]之间的无重复的iCount个随机数
int CRandomCalculator::GetDistinctRandNumber(const int iLow, const int iHigh,
        const int iCount, int aiNumber[])
{
    const int iTotal = iHigh - iLow + 1;
    if (iCount > iTotal)
    {
        return -1;
    }

    int* piTemp = new int[iTotal];
    if (!piTemp)
    {
        return -1;
    }

    int iRange = iTotal;
    int iIdx;
    int i, j;

    for (i = 0; i < iTotal; i++)
    {
        piTemp[i] = i + iLow; // 随机数可能的取值
    }

    for (j = 0; j < iCount; j++)
    {
        iIdx = GetMyRand() % iRange;
        aiNumber[j] = piTemp[iIdx];
        piTemp[iIdx] = piTemp[iRange - 1];
        iRange--;
    }

    delete [] piTemp;
    return 0;
}

// 生成[iLow, iHigh]之间的可重复的iCount个随机数
int CRandomCalculator::GetManyRandNumber(const int iLow, const int iHigh,
        const int iCount, int aiNumber[])
{
    ASSERT_AND_LOG_RTN_INT(aiNumber);

    if (iHigh < iLow)
    {
        return -3;
    }

    const int iTotal = iHigh - iLow + 1;

    int iRange = iTotal;
    int j;

    for (j = 0; j < iCount; j++)
    {
        aiNumber[j] = GetMyRand() % iRange + iLow;
    }

    return 0;
}


// 生成iMin到iMax之间符合二项分布的随机数R ~ B(N， p)
// 其中N=iMax-iMin，且N>10, 固定p=0.5保持对称性
// 可用来模拟正态随机数。此时方差为Np
// 出于性能考虑，建议N介于10到100之间
// 若超过该值，建议将随机区间作等分，将等分值传入，
// 此函数获取的值标识等分区间，然后再在该区间平均随机
// 实现算法：实验方式
int CRandomCalculator::GetBinoRandNum(int iMin, int iMax)
{
    __ASSERT_AND_LOG(iMax >= iMin);
    __ASSERT_AND_LOG(iMin >= 0);

    int iNum = iMax - iMin;
    int iHit = 0;
    for (int i = 0; i < iNum; ++i)
    {
        if ((GetMyRand() % 100) < 50 )
        {
            ++iHit;
        }
    }

    return iMin + iHit;
}

bool CRandomCalculator::TestSuccess(int iSuccessRate)
{
    srand(time(NULL));

    if(iSuccessRate < 0)
    {
        return false;
    }

    if(iSuccessRate >= 100)
    {
        return true;
    }

    int iRadomNumber = GetMyRand() % 100;

    if(iRadomNumber < iSuccessRate)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//根据权重从TotalNum 里选择SelectNum不通物品出来
int CRandomCalculator::GetNotSameRand(int iTotalNum, int iSelectNum, int *aiLoot)
{
    ASSERT_AND_LOG_RTN_INT(aiLoot);
    memset(aiLoot, 0, sizeof(int) * iTotalNum);

    if (iSelectNum >= iTotalNum)
    {
        for (int i = 0; i < iTotalNum; i++)
        {
            aiLoot[i] = 1;
        }
        return 0;
    }

    int aiIdx[iTotalNum];
    for (int i = 0; i < iTotalNum; i++)
    {
        aiIdx[i] = i;
    }

    Shuffle<int>(aiIdx, iTotalNum);

    for (int i = 0; i < iSelectNum; i++)
    {
        aiLoot[aiIdx[i]] = 1;
    }
    return 0;
}

//根据权重从TotalNum 里选择SelectNum个不同物品出来, 存储在aiLoot里
//时间复杂度O(TotalNum * SelectNum)
int CRandomCalculator::GetNotSameRandByWeight(int *aiWeight, int iTotalNum, int iSelectNum, int *aiLoot)
{
    ASSERT_AND_LOG_RTN_INT(aiWeight);
    ASSERT_AND_LOG_RTN_INT(aiLoot);

    memset(aiLoot, 0, sizeof(int) * iTotalNum);

    if (iSelectNum >= iTotalNum)
    {
        for (int i = 0; i < iTotalNum; i++)
        {
            aiLoot[i] = 1;
            return 0;
        }
    }

    int64_t iTotalWeight = 0;
    IdxWeight astWeight[iTotalNum];
    for (int i = 0; i < iTotalNum; i++)
    {
        iTotalWeight += aiWeight[i];
        astWeight[i].iIdx = i;
        astWeight[i].iWeight = aiWeight[i];
    }

    for (int i = 0; i < iSelectNum; i++)
    {
        int iRand = GetRandomNumber(1, iTotalWeight);
        int iWeightSum = 0;
        for (int iSelectIdx = i; iSelectIdx < iTotalNum; iSelectIdx++)
        {
            iWeightSum += astWeight[iSelectIdx].iWeight;
            //落在这个区间了
            if (iRand <= iWeightSum)
            {
                std::swap(astWeight[i], astWeight[iSelectIdx]);
                //总权重减掉已经选中的物品
                iTotalWeight -= astWeight[i].iWeight;
                break;
            }
        }
    }

    for (int i = 0; i < iSelectNum; i++)
    {
        aiLoot[astWeight[i].iIdx] = 1;
    }
    return 0;
}


