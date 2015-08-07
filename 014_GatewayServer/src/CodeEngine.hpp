


#ifndef __CODE_ENGINE_HPP__
#define __CODE_ENGINE_HPP__

#ifndef WIN32
#include <stdint.h>
#endif



int EncodeChar( char **pstrEncode, unsigned char ucSrc );
int DecodeChar( char **pstrDecode, unsigned char *pucDest );
int EncodeShort16( char **pstrEncode, unsigned short usSrc );
int DecodeShort16( char **pstrDecode, unsigned short *pusDest );
int EncodeLong( char **pstrEncode, unsigned long ulSrc );
int DecodeLong( char **pstrDecode, unsigned long *pulDest );
int EncodeInt32( char **pstrEncode, unsigned int uiSrc );
int DecodeInt32( char **pstrDecode, unsigned int *puiDest );
int EncodeString( char **pstrEncode, char *strSrc, short sMaxStrLength );
int DecodeString( char **pstrDecode, char *strDest, short sMaxStrLength );
int EncodeMem( char **pstrEncode, char *pcSrc, short sMemSize );
int DecodeMem( char **pstrDecode, char *pcDest, short sMemSize );

int EncodeMemInt( char **pstrEncode, char *pcSrc, int sMemSize);
int DecodeMemInt( char **pstrDecode, char *pcDest, int sMemSize);

int EncodeInt64( char **pstrEncode, uint64_t uiSrc );
int DecodeInt64( char **pstrDecode, uint64_t *puiDest );



//class CCodeEngine
//{
//public:
	int EncryptData(unsigned short ushAlgorithm,
		const unsigned char* pbyKey,
		const unsigned char* pbyIn,
		short shInLength,
		unsigned char* pbyOut,
		short& rshOutLength);

	int DecryptData(unsigned short ushAlgorithm,
		const unsigned char* pbyKey,
		const unsigned char* pbyIn,
		int iInLength,
		unsigned char* pbyOut,
		int& riOutLength);

	int Compress(unsigned short ushAlgorithm,
		const unsigned char* pbyIn,
		short shInLength,
		unsigned char* pbyOut,
		short* pnOutLength);

	int Uncompress(unsigned short ushAlgorithm,
		const unsigned char* pbyIn,
		short shInLength,
		unsigned char* pbyOut,
		short* pnOutLength);

//};

#endif
