
#ifndef __BILL_WRITER_HPP__
#define __BILL_WRITER_HPP__

#include "LotusDefine.hpp"
#include "ConfigDefine.hpp"

typedef enum enmBillEventID
{
	EBEI_ACCEPT_SOCKET  =   1001, 
	EBEI_CLOSE_SOCKET   =   1002, 
	EBEI_RECV_PACKET    =   1003, 
	EBEI_SEND_PACKET    =   1004, 
    EBEI_DROP_MSG       =   1005, 
    EBEI_CODEQUEUE_MSG  =   1006,
    EBEI_ROUTE_MSG      =   1007,
    EBEI_SERVER_MSG     =   1008,
    EBEI_NOSLAVE_MSG    =   1009,
    EBEI_SLAVE_MSG      =   1010,

} ENMBILLEVENTID;

typedef enum enBillFlag
{
    EBF_ACCEPT_SOCKET   =   0x00000001,
    EBF_CLOSE_SOCKET    =   0x00000002,
    EBF_RECV_PACKET     =   0x00000004,
    EBF_SEND_PACKET     =   0x00000008,
    EBF_DROP_MSG        =   0x00000010,
    EBF_CODEQUEUE_MSG   =   0x00000020,
    EBF_ROUTE_MSG       =   0x00000040,
    EBF_SERVER_MSG      =   0x00000080,
    EBF_NOSLAVE_MSG     =   0x00000100,
    EBF_SLAVE_MSG       =   0x00000200,

} EBillFlag;

class CBillWriter
{
public:
	static int WriteBlockBill(unsigned int uiIP);
	static int WriteSocketBill(TExternalClientSocket* pstClientSocket, short shEventID);

private:
};

#endif


