#ifndef __LOTUS_DEFINE_HPP__
#define __LOTUS_DEFINE_HPP__



//TCP连接控制信息
#define PLAYERNOTIFYTCPCLOSESOCKET  1

const int APPCMD_STOP_SERVICE = 1;
const int APPCMD_RELOAD_CONFIG = 2;

#ifdef _DEBUG_
const unsigned int FD_SIZE = 5000; //方便测试
#else
const unsigned int FD_SIZE = 50000;
#endif

const unsigned int EXTERNALCLIENT_RECVBUFFER_SIZE = 4096;

const unsigned int INTERNALSERVER_RECVBUFFER_SIZE = 2048000;
const unsigned int INTERNALSERVER_SENDBUFFER_SIZE = 2048000;

const unsigned int DISPATCHING_BUFFER_SIZE = 64000;

const unsigned int MAX_VALIDCODE_LENGTH = 32000;

const unsigned int MAX_LISTENER_NUMBER = 16;

const unsigned int PROXYHEAD_BASELENGTH = 14;


const unsigned int MAX_NETHEAD_SIZE = 64;
const unsigned int MIN_NETHEAD_SIZE = 16;

const unsigned int NETHEAD_V2_SIZE = 24;

const unsigned int CRYPT_KEY_LENGTH = 16;

typedef enum enLotusSocketFlag
{
    ELSF_ListeningForExternalClient = (unsigned int)0x00000001,
    ELSF_ListeningForInternalServer = (unsigned int)0x00000002,
    ELSF_ConnectedByExternalClient = (unsigned int)0x00000010,
    ELSF_ConnectedByInternalServer = (unsigned int)0x00000020,
}ELotusSocketFlag;

enum enmPacketToClientFlag
{
    ESCF_NO_PACKET_TO_CLIENT = 0, //Server并未给这个连接回过包
    ESCF_SENT_PACKET_TO_CLIENT = 1, //已经回过包
} ;

typedef struct tagExternalClientSocket
{
    //主机序
    int m_iSocketFD;

    unsigned int m_uiSrcIP;
    unsigned short m_ushSrcPort;
    unsigned int m_uiCreateTime;
    int m_iSendFlag; //标志后台是否给这个客户回包了
    unsigned int m_uiRecvClientTimeStamp; //收到客户端数据的时间戳
    unsigned int m_uiRecvSvrTimeStamp; //收到Svr数据的时间戳

    //网络序
    unsigned int m_uiSocketFD_NBO;
    unsigned int m_uiSrcIP_NBO;
    unsigned short m_ushSrcPort_NBO;
    unsigned int m_uiCreateTime_NBO;

    unsigned int m_uiRecvEndOffset;
    char m_szRecvBuffer[EXTERNALCLIENT_RECVBUFFER_SIZE];

    unsigned int m_uiUin; //由于通知断线的时候需要告诉Uin，所以这里要存下外部连接的Uin

    unsigned int m_uiRecvPacketNumber;

    // Socket链表
    tagExternalClientSocket* m_pPrevSocket;
    tagExternalClientSocket* m_pNextSocket;
}TExternalClientSocket;

typedef struct tagInternalServerSocket
{
    //主机序
    int m_iSocketFD;

    unsigned int m_uiSrcIP;
    unsigned short m_ushListenedPort;

    unsigned short m_ushServerType;
    unsigned short m_ushServerID;

    unsigned int m_uiRecvEndOffset;
    char m_szRecvBuffer[INTERNALSERVER_RECVBUFFER_SIZE];

    unsigned int m_uiSendEndOffset;
    char m_szSendBuffer[INTERNALSERVER_SENDBUFFER_SIZE];

}TInternalServerSocket;

typedef enum enmReserveCtrlFlag
{
    ERCF_LOTUS_NOTIFY_CLIENT_OFFLINE = 1,

} ENMCODECTRLFLAG;

//内部用消息ID，用100以下的ID，不要和现在的消息ID重复
typedef enum enmInternalMsgID
{
    EIMI_LOTUS_NOTIFY_OFFLINE = 1,
} ENMINTERNALMSGID;

typedef struct tagNetHead_V2
{
    unsigned int    m_uiSocketFD;   //套接字
    unsigned int    m_uiSocketTime; //套接字创建时刻
    unsigned int    m_uiSrcIP;  //源地址
    unsigned short  m_ushSrcPort;   //源端口
    unsigned short  m_ushReservedValue01;   //字节对齐，未用
    unsigned int    m_uiCodeTime;   //消息时刻
    unsigned int    m_uiCodeFlag;   //消息标志，用于实现套接字控制
}TNetHead_V2;
#endif
