#ifndef _BASE_HPP_
#define _BASE_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

#ifdef WIN32
#include <winsock2.h>
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <dirent.h>
#include <dlfcn.h>
#endif

#define PRINT xprintf

#define ABS(a,b)	(((unsigned int) (a) > (unsigned int)(b)) ? ((a) - (b)) : ((b) - (a)))
#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define DELETEOBJ(a)  {	delete a ;	a = NULL;}

#define BIT_ENABLED(AWORD, BIT) (((AWORD) & (BIT)) != 0)
#define BIT_DISABLED(AWORD, BIT) (((AWORD) & (BIT)) == 0)
#define SET_BITS(AWORD, BITS) ((AWORD) |= (BITS))
#define CLR_BITS(AWORD, BITS) ((AWORD) &= ~(BITS))

#define SAY nullprintf

#ifndef WIN32
extern FILE* g_fpOutPut;  //PRINT的输出流
#endif

void xprintf(const char *szFormat, ...);
void nullprintf(const char *szFormat, ...);
void PrintBin(char *pBuffer, int iLength);
void HexPrint(void *pBuffer, int iLength, FILE *fpOut = stdout);

//字符串处理函数
void TrimStr( char *strInput );
char* GetNextToken(const char *szSrc, char cSplitter, char *szToken);


//生成随即数的种子
unsigned int GenRandSeed(int iRandomPara = 0);

inline char *GetCurVersion(unsigned int unVer)
{
	static char szVersion[64];
	char szVer[32];
	sprintf(szVer,"%d",unVer);

	sprintf(szVersion," %c.%c ",szVer[0],szVer[1]);
	if (szVer[2] == '0')
	{
	    strcat(szVersion,"Alpha");
	}
	else if (szVer[2] == '1')
	{
	    strcat(szVersion,"Beta");
	}	
	else
	{
	    strcat(szVersion,"Release");
	}
	
	sprintf(szVersion+strlen(szVersion),"%c%c Build%c%c%c",szVer[3],szVer[4],szVer[5],szVer[6],szVer[7]);
	sprintf(szVersion+strlen(szVersion)," (%d)",unVer);

	return &szVersion[0];
}

int CreateDir(const char *szDir);

template <class Type>
inline void Swap(Type &a, Type &b)
{
	Type c = a;
	a = b;
	b = c;
}



#endif
