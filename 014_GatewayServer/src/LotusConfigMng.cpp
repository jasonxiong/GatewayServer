#include "SectionConfig.hpp"
#include "LotusConfigMng.hpp"
#include "LotusLogAdapter.hpp"
#include "base.hpp"
#include "LotusDefine.hpp"

using namespace ServerLib;

CLotusConfigMng::CLotusConfigMng()
{
    m_ushServerType = 0;
    m_ushServerID = 0;
    m_uiInternalSocketBufferSize = DEFAULT_SENDBUFFER_SIZE;
    m_uiClientPacketMaxLength = DEFAULT_INPUTPACKET_LENGTHRESTRCIT;
    m_bIsNeedCheckNetHead = true;

    m_iIdleConnectionTimeout = 0;
    m_iIdleClientTimeout = 0;
    m_iIdleServerTimeout = 0;
    m_iHelloTimeGap = 0;
    m_iClientPacketMaxFrequency = 0;

    m_iCodeQueueNumber = 0;

    m_stClientListenerAddress.m_uiNumber = 0;
    m_stServerListenerAddress.m_uiNumber = 0;

    m_bIsFlashCodeEnabled = false;
}

int CLotusConfigMng::LoadPrimaryConfig()
{
	CSectionConfig stConfigFile;
    int iRet = stConfigFile.OpenFile(FILE_LOTUSCONFIG_SERVER);
    if(iRet != 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "Open File %s Failed.\n", FILE_LOTUSCONFIG_SERVER);
        return -1;
    }

    m_ushServerType = 0;
    int iServerID = 0;
    stConfigFile.GetItemValue("Server", "ServerID", iServerID, -1);
    m_ushServerID = iServerID;

    int iRecordFlag = 0;
    stConfigFile.GetItemValue("Server", "IsRecordEnabled", iRecordFlag, 0);
    m_bIsRecordEnable = iRecordFlag;

    int iFlashFlag = 0;
    stConfigFile.GetItemValue("Server", "IsFlashCodeEnabled", iFlashFlag, 0);
    m_bIsFlashCodeEnabled = iFlashFlag;

    int iSendBufferSize = 0;
    stConfigFile.GetItemValue("Network", "InternalSocketBufferSize", iSendBufferSize, DEFAULT_SENDBUFFER_SIZE);
    m_uiInternalSocketBufferSize = iSendBufferSize;

    stConfigFile.GetItemValue("Network", "ExternalSocketRCVBufferSize", iSendBufferSize, DEFAULT_SENDBUFFER_SIZE);
    m_uiExternalSocketRCVBufferSize = iSendBufferSize;

    stConfigFile.GetItemValue("Network", "ExternalSocketSNDBufferSize", iSendBufferSize, DEFAULT_SENDBUFFER_SIZE);
    m_uiExternalSocketSNDBufferSize = iSendBufferSize;

    int iClientPacketMaxLength = 0;
    stConfigFile.GetItemValue("Network", "ClientPacketMaxLength", iClientPacketMaxLength, DEFAULT_INPUTPACKET_LENGTHRESTRCIT);
    m_uiClientPacketMaxLength = iClientPacketMaxLength;

    stConfigFile.GetItemValue("Network", "IdleConnectionTimeout", m_iIdleConnectionTimeout, 30);

    stConfigFile.GetItemValue("Network", "IdleClientTimeout", m_iIdleClientTimeout, 180);

    stConfigFile.GetItemValue("Network", "IdleServerTimeout", m_iIdleServerTimeout, 1200);

    stConfigFile.GetItemValue("Network", "CheckHelloTimeGap", m_iHelloTimeGap, 10);

    stConfigFile.GetItemValue("Network", "ClientPacketMaxFrequency", m_iClientPacketMaxFrequency, 1000);

    int iNeedCheckNetHead = 0;
    stConfigFile.GetItemValue("Code", "IsNeedCheckNetHead", iNeedCheckNetHead, 1);
    m_bIsNeedCheckNetHead = iNeedCheckNetHead;

    int iBillFlag;
    stConfigFile.GetItemValue("Bill", "BillFlag", iBillFlag, 0);
    m_uiBillFlag = iBillFlag;

    stConfigFile.CloseFile();

    TRACESVR(LOG_LEVEL_ALERT, "ServerType:%d ServerID:%d\n",
        m_ushServerType, m_ushServerID);
    TRACESVR(LOG_LEVEL_ALERT, "InternerBufferSize:%u ExternalRCVBufferSize:%u ExternalSNDBufferSize:%u  ClientPacketMaxLength:%u\n",
        m_uiInternalSocketBufferSize, m_uiExternalSocketRCVBufferSize, m_uiExternalSocketSNDBufferSize, m_uiClientPacketMaxLength);

    TRACESVR(LOG_LEVEL_ALERT, "BillFlag:%u\n",  m_uiBillFlag);

    return 0;
}

int CLotusConfigMng::LoadCodeQueueConfig()
{
    FILE* fpCodeQueueFile = NULL;
    char szLineBuf[1024] = {0};
    int iReadColomns = 0;

    TCodeQueueConfig stTempCQ;

    fpCodeQueueFile = fopen(FILE_LOTUSCONFIG_CODEQUEUE, "r");
    if(!fpCodeQueueFile)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "Open File %s Failed.\n", FILE_LOTUSCONFIG_CODEQUEUE);
        return -1;
    }

    m_iCodeQueueNumber = 0;
    memset(m_astCodeQueueConfig, 0, sizeof(m_astCodeQueueConfig));
    memset(m_aiCodeQueueIDToIndex, 0, sizeof(m_aiCodeQueueIDToIndex));

    while(!feof(fpCodeQueueFile))
    {
        memset(szLineBuf, 0, sizeof(szLineBuf));
        memset(&stTempCQ, 0, sizeof(stTempCQ));

        fgets((char *)szLineBuf, sizeof(szLineBuf), fpCodeQueueFile);
        TrimStr((char *)szLineBuf);
        if(szLineBuf[0] == '#')
        {
            continue;
        }

        iReadColomns = sscanf(szLineBuf, "%d%s%s%u%u%u",
            &stTempCQ.m_iCodeQueueID,
            stTempCQ.m_szKeyFileName, &stTempCQ.m_ucKeyPrjID,
            &stTempCQ.m_uiShmSize, &stTempCQ.m_uiCodeQueueType, &stTempCQ.m_uiCodeQueueFlag);

        if(iReadColomns < 6)
        {
            continue;
        }

        if(!IsCodeQueueAvailable(stTempCQ))
        {
            continue;
        }

        TCodeQueueConfig& rstCodeQueueConfig = m_astCodeQueueConfig[m_iCodeQueueNumber];
        rstCodeQueueConfig = stTempCQ;
        m_aiCodeQueueIDToIndex[rstCodeQueueConfig.m_iCodeQueueID] = m_iCodeQueueNumber;
        m_iCodeQueueNumber++;
    }

    TRACESVR(LOG_LEVEL_DETAIL, "In CodeConfig CodeQueueNumber:%d\n", m_iCodeQueueNumber);
    int i;
    for(i = 0; i < m_iCodeQueueNumber; ++i)
    {
        TCodeQueueConfig& rstCodeQueueConfig = m_astCodeQueueConfig[i];
        TRACESVR(LOG_LEVEL_DETAIL, "CodeQueue[%d]:CodeQueueID:%d CodeQueueFlag:%u CodeQueueFile:%s CodeQueuePrj:%c CodeQueueSize:%u\n",
            i, rstCodeQueueConfig.m_iCodeQueueID, rstCodeQueueConfig.m_uiCodeQueueFlag,
            rstCodeQueueConfig.m_szKeyFileName, rstCodeQueueConfig.m_ucKeyPrjID,
            rstCodeQueueConfig.m_uiShmSize);
    }

    fclose(fpCodeQueueFile);

    return 0;
}

int CLotusConfigMng::LoadUinConfig()
{
    FILE* fUinFile = NULL;
    char szLineBuf[1024] = {0};
    fUinFile = fopen(FILE_LOTUSCONFIG_TRACE_UIN, "r");

    if(fUinFile == NULL)
    {
        CLEARLOCALTRACEFLAG();
        return 0;
    }

    CLEARLOCALTRACEFLAG();
    unsigned int uiTempUin = 0;
    int iColumns = 0;

    short shTraceUinNum = 0;

    while(true)
    {
        fgets(szLineBuf, sizeof(szLineBuf), fUinFile);
        if(feof(fUinFile))
        {
            break;
        }

        TrimStr(szLineBuf);

        if(szLineBuf[0] == '#')
        {
            continue;
        }

        iColumns = sscanf(szLineBuf, "%u", &uiTempUin);
        if(iColumns < 1 || uiTempUin < 10000)
        {
            continue;
        }

        TRACESVR(LOG_LEVEL_DETAIL, "TracedUin[%d] = %u.\n", shTraceUinNum, uiTempUin);
        ADDGLOBALPLAYERUINFLAG(uiTempUin);
        shTraceUinNum++;
    }

    fclose(fUinFile);

    return 0;
}

int CLotusConfigMng::LoadFlashConfig()
{
    FILE * fpFlashConfig = NULL;

    fpFlashConfig = fopen(FILE_LOTUSCONFIG_FLASH_REQ, "r");
    if( !fpFlashConfig )
    {
        TRACESVR(LOG_LEVEL_DETAIL, "Load Flash Req File Failed\n");
        return -1;
    }

    memset(&m_stFlashReqConfigBuffer, 0, sizeof(m_stFlashReqConfigBuffer));

    short shBytes = fread((void *)(m_stFlashReqConfigBuffer.m_szContent), 1, POLICY_FILE_SIZE, fpFlashConfig);
    if (shBytes <= 0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CFlashSvrCtrl::LoadConfig, shBytes(fread())=%d\n", shBytes);
        return -2;
    }

    TrimStr(m_stFlashReqConfigBuffer.m_szContent);
    m_stFlashReqConfigBuffer.m_shLength = strlen(m_stFlashReqConfigBuffer.m_szContent);

    fclose(fpFlashConfig);
    fpFlashConfig = NULL;

    TRACESVR(LOG_LEVEL_DETAIL, "Flash Req (%d) Bytes:%s\n", m_stFlashReqConfigBuffer.m_shLength, m_stFlashReqConfigBuffer.m_szContent);

    fpFlashConfig = fopen(FILE_LOTUSCONFIG_FLASH_RSP, "r");
    if( !fpFlashConfig )
    {
        TRACESVR(LOG_LEVEL_DETAIL, "Load Flash Rsp File Failed\n");
        return -3;
    }

    memset(&m_stFlashRspConfigBuffer, 0, sizeof(m_stFlashRspConfigBuffer));

    shBytes = fread((void *)(m_stFlashRspConfigBuffer.m_szContent), 1, POLICY_FILE_SIZE, fpFlashConfig);
    if (shBytes<=0)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "In CFlashSvrCtrl::LoadConfig, shBytes(fread())=%d\n", shBytes);
        return -4;
    }

    TrimStr(m_stFlashRspConfigBuffer.m_szContent);
    m_stFlashRspConfigBuffer.m_shLength = strlen(m_stFlashRspConfigBuffer.m_szContent);

    TRACESVR(LOG_LEVEL_DETAIL, "Flash Rsp (%d) Bytes:%s\n", m_stFlashRspConfigBuffer.m_shLength, m_stFlashRspConfigBuffer.m_szContent);

    fclose(fpFlashConfig);

    return 0;
}

int CLotusConfigMng::LoadListenerConfig()
{
    FILE* fListenerFile = NULL;
    char szLineBuf[1024] = {0};
    int iReadColomns = 0;

    fListenerFile = fopen(FILE_LOTUSCONFIG_LISTENER, "r");
    if(!fListenerFile)
    {
        TRACESVR(LOG_LEVEL_DETAIL, "Open file %s Failed.\n", FILE_LOTUSCONFIG_LISTENER);
        return -1;
    }

    m_stClientListenerAddress.m_uiNumber = 0;
    m_stServerListenerAddress.m_uiNumber = 0;


    while(!feof(fListenerFile)
        && m_stClientListenerAddress.m_uiNumber < MAX_LISTEN_PORT_NUMBER
        && m_stServerListenerAddress.m_uiNumber < MAX_LISTEN_PORT_NUMBER)
    {
        memset(szLineBuf, 0, sizeof(szLineBuf));

        unsigned int uiType = 0;
        char szListenerIP[MAX_IPV4_LENGTH] = {0};
        unsigned int uiIP = 0;
        unsigned int uiPort = 0;

        fgets((char *)szLineBuf, sizeof(szLineBuf), fListenerFile);
        TrimStr((char *)szLineBuf);
        if(szLineBuf[0] == '#')
        {
            continue;
        }

        iReadColomns = sscanf(szLineBuf, "%u%s%u",
            &uiType, szListenerIP, &uiPort);
        if(iReadColomns < 3)
        {
            continue;
        }

        uiIP = inet_addr(szListenerIP);

        if(uiType == ESLT_ExternalClinet)
        {
            TAddress& rstAddress = m_stClientListenerAddress.m_astAddress[m_stClientListenerAddress.m_uiNumber];
            rstAddress.m_uiIP = uiIP;
            rstAddress.m_ushPort = uiPort;
            m_stClientListenerAddress.m_uiNumber++;
        }
        else if(uiType == ESLT_InternalServer)
        {
            TAddress& rstAddress = m_stServerListenerAddress.m_astAddress[m_stServerListenerAddress.m_uiNumber];
            rstAddress.m_uiIP = uiIP;
            rstAddress.m_ushPort = uiPort;
            m_stServerListenerAddress.m_uiNumber++;
        }
    }

    int i = 0;
    for(i = 0; i < (int)m_stClientListenerAddress.m_uiNumber; ++i)
    {
        TAddress& rstAddress = m_stClientListenerAddress.m_astAddress[i];
        TRACESVR(LOG_LEVEL_DETAIL, "ClientListener[%d] IP:%u Port:%u\n", i, rstAddress.m_uiIP, rstAddress.m_ushPort);
    }

    for(i = 0; i < (int)m_stServerListenerAddress.m_uiNumber; ++i)
    {
        TAddress& rstAddress = m_stServerListenerAddress.m_astAddress[i];
        TRACESVR(LOG_LEVEL_DETAIL, "ServerListener IP[%d]:%u Port:%u\n", i, rstAddress.m_uiIP, rstAddress.m_ushPort);
    }

    fclose(fListenerFile);

    return 0;
}

const TAddressGroup& CLotusConfigMng::GetExternalListenerAddress()
{
    return m_stClientListenerAddress;
}

const TAddressGroup& CLotusConfigMng::GetInternalListenerAddress()
{
    return m_stServerListenerAddress;
}

const int CLotusConfigMng::GetCodeQueueNumber()
{
    return m_iCodeQueueNumber;
}

const TCodeQueueConfig& CLotusConfigMng::GetCodeQueueByIndex(int iCodeQueueIndex)
{
    return m_astCodeQueueConfig[iCodeQueueIndex];
}

const TCodeQueueConfig& CLotusConfigMng::GetCodeQueueByID(int iCodeQueueID)
{
    int iCodeQueueIndex = m_aiCodeQueueIDToIndex[iCodeQueueID];

    return m_astCodeQueueConfig[iCodeQueueIndex];
}

bool CLotusConfigMng::IsCodeQueueAvailable(TCodeQueueConfig& rstCodeQueueConfig)
{
    if(rstCodeQueueConfig.m_iCodeQueueID < 0)
    {
        return false;
    }

    return true;
}

unsigned short CLotusConfigMng::GetServerType()
{
    return m_ushServerType;
}

unsigned short CLotusConfigMng::GetServerID()
{
    return m_ushServerID;
}

int CLotusConfigMng::GetIdleConnectionTimeout()
{
    return m_iIdleConnectionTimeout;
}

int CLotusConfigMng::GetIdleClientTimeout()
{
    return m_iIdleClientTimeout;
}

int CLotusConfigMng::GetIdleServerTimeout()
{
    return m_iIdleServerTimeout;
}

unsigned int CLotusConfigMng::GetInternalBufferSize()
{
    return m_uiInternalSocketBufferSize;
}

unsigned int CLotusConfigMng::GetExternalRCVBufferSize()
{
    return m_uiExternalSocketRCVBufferSize;
}

unsigned int CLotusConfigMng::GetExternalSNDBufferSize()
{
    return m_uiExternalSocketSNDBufferSize;
}

unsigned int CLotusConfigMng::GetClientPacketMaxLength()
{
    return m_uiClientPacketMaxLength;
}

bool CLotusConfigMng::IsRecordEnabled()
{
    return m_bIsRecordEnable;
}

bool CLotusConfigMng::IsFlashCodeEnabled()
{
    return m_bIsFlashCodeEnabled;
}

const TMsgConfigBuffer& CLotusConfigMng::GetFlashReqBuf()
{
    return m_stFlashReqConfigBuffer;
}

const TMsgConfigBuffer& CLotusConfigMng::GetFlashRspBuf()
{
    return m_stFlashRspConfigBuffer;
}

int CLotusConfigMng::GetHelloTimeGap()
{
    return m_iHelloTimeGap;
}

int CLotusConfigMng::GetClientPacketMaxFrequencyNum()
{
    return m_iClientPacketMaxFrequency;
}

unsigned int CLotusConfigMng::GetBillFlag()
{
    return m_uiBillFlag;
}


