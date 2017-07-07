/**
*@file  CommonDef.hpp
*@author jasonxiong
*@date 2009-07-02
*@version 1.0
*@brief 该文件包括一些基础的宏定义
*
*
*/

#ifndef __COMMON_DEF_HPP__
#define __COMMON_DEF_HPP__

#include "stddef.h"

namespace ServerLib
{
	const unsigned int MAX_FILENAME_LENGTH = 256; //!<文件名最大长度
	const unsigned int MAX_DATETIME_LENGTH = 32; //!<YYYY-mm-dd HH:MM:SS格式的时间串最大长度
	const unsigned int MAX_DATE_LENGTH = 16; //!<YYYY-mm-dd格式的时间串最大长度
	const unsigned int MAX_IPV4_LENGTH = 16; //!<IPv4的字符串长度
	const int MIN_TIME_STRING_LENGTH = 19; //!<用于转换时存放tm时间字符串
	const int MAX_LINE_BUF_LENGTH = 1024; //!<一行字符串的最大长度

	const int MICROSECOND_PER_SECOND = 1000000; //!<每秒有多少微秒

//!计算|a-b|
#define ABS(a,b)  (((unsigned int) (a) > (unsigned int)(b)) ? ((a) - (b)) : ((b) - (a)))

//!获取a和b的较大者
#ifndef MAX
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#endif

	//!获取a和b的较小者
#ifndef MIN
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#endif

	//!安全释放指针
#define DELETEOBJ(a)  {	if(a) { delete a; } a = NULL; }

	//!判断AWORD是否置上某一个BIT
#define BIT_ENABLED(AWORD, BIT) (((AWORD) & (BIT)) != 0)

	//!判断AWORD是否没置上某一个BIT
#define BIT_DISABLED(AWORD, BIT) (((AWORD) & (BIT)) == 0)

	//!将AWORD置上某些BITS
#define SET_BITS(AWORD, BITS) ((AWORD) |= (BITS))

	//!将AWORD清除某些BITS
#define CLR_BITS(AWORD, BITS) ((AWORD) &= ~(BITS))

	template <typename T, size_t N>
	char(&ArraySizeHelper(T(&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))


	//按位操作辅助宏
#define cast(t, exp)	((t)(exp))
#define resetbits(x,m)	((x) &= (~(m)) )
#define setbits(x,m)	((x) |= (m))
#define testbits(x,m)	((x) & (m))
#define bitmask(b)	(1<<(b))

	//设置字节x的第b位为1
#define setbitsbypos(x, b)		setbits(x, bitmask(b))
#define resetbitsbypos(x, b)	resetbits(x, bitmask(b));
}

#endif //__COMMON_DEF_HPP__
