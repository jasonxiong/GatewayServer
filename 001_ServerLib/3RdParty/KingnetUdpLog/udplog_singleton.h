 

#ifndef _UDPLOG_SINGLETON_H_
#define _UDPLOG_SINGLETON_H_


#include <stdio.h>


template<typename T>
class CKingnetUdpLogSingleton
{
public:
	//创建实例
    static T& CreateInstance()
	{
		return GetInstance();
	}
	//获取实例
	static T& GetInstance()
	{
		if (NULL == ms_pInstance)
		{
			ms_pInstance = new T();
		}

		return *ms_pInstance;
	}
	//销毁实例
	static void DestroyInstance()
	{
		if (NULL != ms_pInstance)
		{
			delete ms_pInstance;
			ms_pInstance = NULL;
		}
	}

	//将用新的实例将旧的实例换出来
	static T* SwapInstance(T* new_one)
	{
		T* old_one = ms_pInstance;
		ms_pInstance = new_one;
		return old_one;
	}

protected:
	static T	*ms_pInstance;

protected:
	CKingnetUdpLogSingleton()
	{
	}
	virtual ~CKingnetUdpLogSingleton()
	{
	}

private:
    // Prohibited actions
    CKingnetUdpLogSingleton(const CKingnetUdpLogSingleton &);
    CKingnetUdpLogSingleton& operator = (const CKingnetUdpLogSingleton &);

};

template<typename T>
T* CKingnetUdpLogSingleton<T>::ms_pInstance = NULL;


#endif
