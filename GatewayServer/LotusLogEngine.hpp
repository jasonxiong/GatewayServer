
#ifndef __LOTUS_LOGENGINE_HPP__
#define __LOTUS_LOGENGINE_HPP__

//#include <string.h>
#include <stdarg.h>

#include "GlobalValue.hpp"
#include "SingletonTemplate.hpp"

//定义所有程序可能共享的日志级别(预留一个字节） 0x0001-0x00000080

#define LOG_LEVEL_DEBUG         0x00000001
#define LOG_LEVEL_INFO          0x00000002
#define LOG_LEVEL_NOTICE        0x00000004
#define LOG_LEVEL_WARNING       0x00000008
#define LOG_LEVEL_ERROR         0x00000010
#define LOG_LEVEL_CRISIS        0x00000020
#define LOG_LEVEL_ALERT         0x00000040
#define LOG_LEVEL_EMERGENCY     0x00000080

#define LOG_LEVEL_CALL          0x00000100      //跟踪函数调用
#define LOG_LEVEL_DETAIL        0x00000800      //详细日志

#define LOG_LEVEL_NONE          0x00000000
#define LOG_LEVEL_ANY           0xFFFFFFFF

class CLotusLogEngine
{
public:
    CLotusLogEngine();
    virtual ~CLotusLogEngine();

    int InitLog(
        char *pcFileName,
        int iMaxFileSize, int iMaxFileCount);

    int WriteLog(
        char *pcErrInfo,
        const char *pcContent, ...);

    int WriteLog(
        char *pcFileName,
        int iMaxFileSize, int iMaxFileCount,
        char *pcErrInfo,
        const char *pcContent, va_list ap);

    int WriteLog(
        char *pcFileName,
        char *pcErrInfo,
        const char *pcContent, va_list ap);

    int PrintBin(char *pcFileName,
        char *pcBuffer,
        int iLength,
        char *pcErrInfo);

    int CheckLogLevel(unsigned int uiLogLevel);

    int SetLogLevel(unsigned int uiLogLevel);

    int ShiftFile(char *pcFileName,
        int iMaxFileSize, int iMaxFileCount,
        char *pcErrInfo);



private:

    unsigned int m_uiLogLevel;

    char m_szFileName[MAX_FILENAME_LENGTH];
    int m_iMaxFileSize;
    int m_iMaxFileCount;

    int m_iSimpleTrace;

};



#endif /* __LOGENGINE_HPP__ */


