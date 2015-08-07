 
#ifndef _COMMON_TYPEDEF_H_
#define _COMMON_TYPEDEF_H_


#ifdef __GNUC__
#include <stdint.h>
#include <inttypes.h>
#endif

#if defined(WIN32) || defined(_WIN32)
typedef char						int8_t;
typedef unsigned char				uint8_t;
typedef short						int16_t;
typedef unsigned short				uint16_t;
typedef int							int32_t;
typedef unsigned int				uint32_t;
typedef long long					int64_t;
typedef unsigned long long			uint64_t;
#elif defined(_WIN64)
typedef char						int8_t;
typedef unsigned char				uint8_t;
typedef short						int16_t;
typedef unsigned short				uint16_t;
typedef int							int32_t;
typedef unsigned int				uint32_t;
typedef long 						int64_t;
typedef unsigned long				uint64_t;
#endif

#define MAX_LOG_LENGTH  1024
#define MAX_MESSAGE_LENGTH MAX_LOG_LENGTH*5
#define MAX_OPTION_LENGTH  128
#define S_OK 0
#define E_FAIL -1
#define MAX_IPADDR_LENGTH 20
#define MAX_USERMSG_LENGTH 512

#define DEFAULT_CONFIGFILENAME			"./config/udplog.xml"
//#define DEFAULT_CONFIGFILENAME			"D:\\udp_log\\udplog_cpp\\udplog_cpp\\udplog_cpp\\Debug\\config\\udplog.xml"

//#if defined(WIN32) || defined(_WIN32)
//#pragma warning(disable:4996)
//#pragma warning(disable:4005)
//#endif
//
//#if defined(WIN32) || defined(_WIN32)
//#define OS_WIN32 1;
//#else
//#define OS_LINUX 1;
//#endif

#endif
