#ifndef __PREDECODER_HPP__
#define __PREDECODER_HPP__



const unsigned short AUTH_FAILED_MSG_ID = 0xFFFE;
const unsigned short AUTH_ERROR_ERROR_SKEY = 1001;
const int AUTH_ERROR_STR_NUMBER = 256;



class CAuthFailedMsg
{
public:
	int Encode(char *pszOutBuffer, short& rshOutLength);

public:
	unsigned short m_ushMsgID;  // 固定消息ID
	unsigned short m_ushResultID; // 错误码
	unsigned short m_ushErrStrNumber; // 错误字符串长度
	char m_szErrStr[AUTH_ERROR_STR_NUMBER]; // 错误提示

};

#endif
