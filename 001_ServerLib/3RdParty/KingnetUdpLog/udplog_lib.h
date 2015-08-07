#ifndef _UDPLOG_LIB_H_
#define _UDPLOG_LIB_H_


#include "udplog_codeengine.h"

class IMessageBody
{
public:
	virtual int32_t Encode(uint8_t* buf, uint32_t size, uint32_t& offset) = 0;
	virtual int32_t Decode(const uint8_t* buf, const uint32_t size, uint32_t& offset) = 0;
	virtual int32_t Dump(char* buf, const uint32_t size, uint32_t& offset) = 0;
};

class CCSNotifyWriteLog :public IMessageBody
{
public:
	int32_t m_nUid;
	int16_t m_nTableType;
	char	m_szLog[MAX_LOG_LENGTH];
public:
	CCSNotifyWriteLog();
	virtual ~CCSNotifyWriteLog();

	int32_t Encode(uint8_t* buf, uint32_t size, uint32_t& offset);
	int32_t Decode(const uint8_t* buf, const uint32_t size, uint32_t& offset);
	int32_t Dump(char* buf, const uint32_t size, uint32_t& offset);
};


class CCSHead :public IMessageBody
{
public:
	uint8_t m_nHttpHead;
	int32_t m_nPackageLength;
	int32_t m_nUid;
	int16_t m_nFlag;
	int16_t m_nOptionLength;
	uint8_t	m_szOption[MAX_OPTION_LENGTH];
	int16_t m_nHeaderLength;
	int16_t m_nMessageID;
	int16_t m_nMessageType;
	int16_t m_nVersionType;
	int16_t m_nVersion;
	int32_t m_nResourceID;
	int32_t m_nSendTime;

public:
	CCSHead();
	virtual ~CCSHead();

	int32_t Encode(uint8_t* buf, uint32_t size, uint32_t& offset);
	int32_t Decode(const uint8_t* buf, const uint32_t size, uint32_t& offset);
	int32_t Dump(char* buf, const uint32_t size, uint32_t& offset);
};

#endif
