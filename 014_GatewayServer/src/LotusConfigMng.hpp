#ifndef __LOCALCONFIGMNG_HPP__
#define __LOCALCONFIGMNG_HPP__

#include "GlobalValue.hpp"
#include "ConfigDefine.hpp"

class CLotusConfigMng
{
public:
    CLotusConfigMng();

public:

    int LoadPrimaryConfig();
    int LoadCodeQueueConfig();
    int LoadListenerConfig();
    int LoadUinConfig();
    int LoadFlashConfig();

    unsigned short GetServerType();
    unsigned short GetServerID();
    int GetIdleConnectionTimeout();
    int GetIdleClientTimeout();
    int GetIdleServerTimeout();
    int GetHelloTimeGap();
    int GetClientPacketMaxFrequencyNum();
    unsigned int GetInternalBufferSize();
    unsigned int GetExternalRCVBufferSize();
    unsigned int GetExternalSNDBufferSize();
    unsigned int GetClientPacketMaxLength();
    unsigned int GetBillFlag();

    bool IsRecordEnabled();
    bool IsNeedCheckNetHead() { return m_bIsNeedCheckNetHead; }

    bool IsFlashCodeEnabled();
    const TMsgConfigBuffer& GetFlashReqBuf();
    const TMsgConfigBuffer& GetFlashRspBuf();

    const TAddressGroup& GetExternalListenerAddress();
    const TAddressGroup& GetInternalListenerAddress();

    const int GetCodeQueueNumber();
    const TCodeQueueConfig& GetCodeQueueByIndex(int iCodeQueueIndex);
    const TCodeQueueConfig& GetCodeQueueByID(int iCodeQueueID);

private:
    //判断指定CodeQueue配置是否可用
    bool IsCodeQueueAvailable(TCodeQueueConfig& rstCodeQueueConfig);
private:
    //Primary
    unsigned short m_ushServerType;
    unsigned short m_ushServerID;

    //内部缓冲区尺寸
    unsigned int m_uiInternalSocketBufferSize;

    // 外部缓冲区大小
    // SO_RCVBUF
    unsigned int m_uiExternalSocketRCVBufferSize;
    // SO_SNDBUF
     unsigned int m_uiExternalSocketSNDBufferSize;

    //单次上行报文长度上限
    unsigned int m_uiClientPacketMaxLength;

    //检查超时的时间
    int m_iIdleConnectionTimeout; //连接超时时间
    int m_iIdleClientTimeout; //没有收到客户端包超时时间
    int m_iIdleServerTimeout; //没有收到Svr回包的超时时间

    //Hello包检查时间
    int m_iHelloTimeGap;

    //上行请求频率限制
    int m_iClientPacketMaxFrequency;

    //是否根据NetHead和连接进行效验
    bool m_bIsNeedCheckNetHead;

    //账单标志,标记需要打印什么账单
    unsigned int m_uiBillFlag;

    //外部监听地址集合
    TAddressGroup m_stClientListenerAddress;
    //内部监听地址集合
    TAddressGroup m_stServerListenerAddress;

    //CodeQueue数量
    int m_iCodeQueueNumber;
    //CodeQueue配置
    TCodeQueueConfig m_astCodeQueueConfig[MAX_CODEQUEUE_NUMBER];
    int m_aiCodeQueueIDToIndex[MAX_CODEQUEUE_NUMBER];

    //Flash安全策略开关
    bool m_bIsFlashCodeEnabled;

    //Flash 策略请求和相应
    TMsgConfigBuffer m_stFlashReqConfigBuffer;
    TMsgConfigBuffer m_stFlashRspConfigBuffer;

    //默认录制开关
    bool m_bIsRecordEnable;
};

#endif
