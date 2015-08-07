
#include "CodeEngine.hpp"
#include "PreDecoder.hpp"



int CAuthFailedMsg::Encode(char *pszOutBuffer, short& rshOutLength)
{
	char *pcTemp;
	short sTempLength;

	if( !pszOutBuffer )
	{
		return -1;
	}

	pcTemp = pszOutBuffer;
	rshOutLength = 0;

	sTempLength = EncodeShort16( &pcTemp, (unsigned short)m_ushMsgID );
	rshOutLength += sTempLength;

	sTempLength = EncodeShort16( &pcTemp, (unsigned short)m_ushResultID );
	rshOutLength += sTempLength;

	sTempLength = EncodeString(&pcTemp, m_szErrStr, AUTH_ERROR_STR_NUMBER - 1);    
	rshOutLength += sTempLength;

	return 0;
}


