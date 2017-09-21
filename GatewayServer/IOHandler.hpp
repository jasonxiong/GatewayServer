#ifndef __IO_HANDLER_HPP__
#define __IO_HANDLER_HPP__

#include "EpollWrapper.hpp"
#include "LotusListener.hpp"
#include "FDPool.hpp"
#include "CodeQueueAssemble.hpp"
#include "ConfigAssemble.hpp"
#include "CodeDispatcher.hpp"
#include "OverloadSafeguard.hpp"

enum ENU_RESETERROR
{
    TIMEOUT_SVR_NOT_RSP = 1,                   // 空闲连接超时，且Server并未给这个连接回过包(closed by not having Server rsp)
    TIMEOUT_IDLE_CLIENT = 2,                   // 空闲client超时(closed by not recving message too long)
    TIMEOUT_IDLE_SVR    = 3,                   // 空闲server超时(closed by Server not rsp message too long)
    ERROR_NET_IO        = 4,                   // 响应网络IO错误处理
    ERROR_INTERNAL_CONN = 5,                   // 内部连接套接字(必须关闭)
    ERROR_INTERNAL_IO_READ = 6,                // 响应网络IO读，来自内部连接,Recv < 0
    ERROR_DISPATCH_FROM_INTERNAL_SVR = 7,      // DispatchFromInternalServer return < 0
    ERROR_EXTERAL_RECV_REQ_OVERLOAD = 8,       // OnReadFromExternal, RecvReqLimit Overload, Close Socket
    ERROR_EXTERAL_ACCEPT_RECV_OVERLOAD = 9,    // OnReadFromExternal, AcceptReceive Overload, Close Socket
    ERROR_EXTERAL_RECV = 10,                   // 接收失败或者玩家关闭
    ERROR_EXTERAL_RECV_PACKET_TOO_MORE = 11,   // 单个包长大于限制
    RSP_FLASH_STRATEGY_FILE = 12,              // 下发Flash策略文件OK   
    RSP_U3D_STRATEGY_FILE = 13,                // 下发U3D策略文件OK
    ERROR_DISPATCH_FROM_EXTERNAL_CLIENT = 14,  // OnReadFromExternal, DispatchFromExternalClient fail
    SVR_NOTIFY_CLOSE_FD = 15,                  // SendToExternalClient, Svr Notify TO Close Socket
    ERROR_EXTERAL_SEND_TO_CLIENT_FAIL = 16,    // SendToExternalClient Error Send
    ERROR_PLAYERNOTIFYTCPCLOSESOCKET = 17,           // SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET
    ERROR_EXTERAL_SEND_CLIENT_1 = 18,          // SendToExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_2 = 19,          // SendToExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_3 = 20,          // SendToExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_4 = 21,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_5 = 22,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_6 = 23,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_7 = 24,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_8 = 25,          // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_9 = 26,          // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_10 = 27,         // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_11 = 28,         // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_12 = 29,         // SendToExternalClientList, Error Send
    ERROR_EXTERAL_SEND_CLIENT_13 = 30,         // SendToExternalClientList, Error Send
    ERROR_EXTERAL_SEND_CLIENT_14 = 31,         // SendToExternalClientList, Error Send
    ERROR_EXTERAL_SEND_CLIENT_15 = 32,         // SendToExternalClientList, Error Send
    PLAYERNOTIFYTCPCLOSESOCKET_2 = 33,         // SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET
    ERROR_EXTERAL_SEND_CLIENT_16 = 34,         // SendToExternalClient, Error Send
    ERROR_AUTH_FAILED_WHILE_Initing = 35,      // ProcessAuthCodeQueue, Auth Failed While Initing, Reset
    INVALID_AUTHORIZE_PACKET = 36,             // DispatchOneCode, Invalid Authorized packet
    SEND_TO_AUTH_FAIL = 37,                    // Send To Auth failed
    SEND_TO_INTERNAL_SVR_FAIL = 38,            // DispatchToDstServer,SendToInternalServer fail
};

class CIOHandler
{
public:

    //初始化配置集合、
    int Initialize(bool bNagelOff);

    //创建网络IO处理器，本版本只支持Epoll
    int CreateEpoll();

    //创建所有监听器
    int CreateAllListener();

    //创建一枚监听器
    int CreateOneListener(unsigned int uiIP, unsigned short ushPort,
        unsigned int uiSendBufferSize, bool bExternalClient);

    CLotusListener* GetListenerByFD(int iListeningSocket);
    int ReCreateOneListener(CLotusListener *pstLotusListener);

    //检查网络IO
    int CheckIO();

    //检查超时情况
    int CheckTimeOut();
    int CheckOffline(); //检查断线

    int DispatchFromInternalServer(int iSrcFD);

    //检查进程通信CodeQueue
    int CheckCodeQueue();

    int CheckDefaultCodeQueue();

    int SendToExternalClient(const unsigned short ushCodeLength, const char* pszCodeBuffer);

    int SendToExternalClient(const unsigned short ushCodeLength, const char* pszCodeBufBeg, const char* pszCodeBufEnd,
        const char* pszHeadCode, const char* pszTailCode);
    int SendToExternalClientList(const unsigned short ushCodeLength, const char* pszCodeBufBeg, const char* pszCodeBufEnd,
        const char* pszHeadCode, const char* pszTailCode);
    int SendToAllExternalClient(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
        const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode);
    int SendToAllExternalClientButOne(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
        const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode);

    int PrintStat();

    CCodeQueueAssemble& GetCodeQueueAssemble() {return m_stCodeQueueAssemble;}

public:
    //响应网络IO错误处理
    static int OnError(int iFD);
    //响应网络IO读
    static int OnRead(int iFD);
    //响应网络IO写
    static int OnWrite(int iFD);

    int AddFD(int iFD);

    //重置连接
    int Reset(int iFD, int iErrorNo, bool bNotSendOffline = false);

    //设置应用命令，停止服务、重载配置
    static void SetAppCmd(const int iAppCmd);

    //检查应用命令，停止服务、重载配置
    void CheckAppCmd();

    //停止服务
    void StopService();

    //重载配置
    void ReloadConfig();

    //通知断线
    int NotifyInternalMsg(int iFD, unsigned int uiUin, unsigned short ushCtrlFlag = 0);

    //断开Server配置改变的内部连接
    int ResetInternalServer();

private:
    // 正常数据包处理
    int DispatchFromExternalClient(int iFD);

    //处理连接请求
    int Accept(int iListeningFD);

    //处理连接请求时的
    int OnAccept(unsigned int uiIP);

    //响应网络IO读，来自内部连接
    int OnReadFromInternal(int iFD);

    //响应网络IO读，来自外部连接
    int OnReadFromExternal(int iFD);

private:

    //FD池所用的共享内存实例
    CSharedMemory m_stSharedMemoryFDPool;

    //FD池句柄
    CFDPool* m_pstFDPool;

    //网络IO处理器，本版本只支持Epoll实例
    CEpollWrapper m_stEpollWrapper;

    //监听器实例
    unsigned int m_uiListenerNumber;
    CLotusListener m_astLotusListener[MAX_LISTENER_NUMBER];

    //CodeQueue集合
    CCodeQueueAssemble m_stCodeQueueAssemble;

    //配置配置
    CConfigAssemble m_stConfigAssemble;

    //消息派发器
    CCodeDispatcher m_stCodeDispatcher;

    //应用命令
    static int m_iAppCmd;

    time_t m_tLastCheckTimeout; //上一次检查超时的时间
    time_t m_tLastCheckHello;//上次检查Hello时间

    //负载保护
    time_t m_tLastSafeguradTimeout;
    COverloadSafeguard m_stSafegurad; //过载保护类
    COverloadSafeguardStrategy m_stSafeguradStrategy; //过载保护策略类

private:

    static CIOHandler* m_pstThisHandler;
    bool m_bNagelOff; // 收发套接字是否关闭nagel算法

};





#endif
