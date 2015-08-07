#include "ConfigAssemble.hpp"



CConfigAssemble::CConfigAssemble()
{

}

int CConfigAssemble::LoadConfig()
{
    int iRet = 0;

    //主配置
    iRet = m_stLotusConfigMng.LoadPrimaryConfig();
    if(iRet < 0)
    {
        return -3;
    }

    //CodeQueue配置
    iRet = m_stLotusConfigMng.LoadCodeQueueConfig();
    if(iRet < 0)
    {
        return -4;
    }

    //跟踪日志Uin
    m_stLotusConfigMng.LoadUinConfig();

    //监听端口
    iRet = m_stLotusConfigMng.LoadListenerConfig();
    if(iRet < 0)
    {
        return -5;
    }

    //Flash策略文件
    if(m_stLotusConfigMng.IsFlashCodeEnabled())
    {
        iRet = m_stLotusConfigMng.LoadFlashConfig();
        if(iRet < 0)
        {
            return -6;
        }
    }

    return 0;
}

int CConfigAssemble::ReloadConfig()
{
    int iRet = 0;

    //主配置
    iRet = m_stLotusConfigMng.LoadPrimaryConfig();
    if(iRet < 0)
    {
        return -4;
    }

    //CodeQueue配置
    iRet = m_stLotusConfigMng.LoadCodeQueueConfig();
    if(iRet < 0)
    {
        return -5;
    }

    //跟踪日志Uin
    m_stLotusConfigMng.LoadUinConfig();

    return 0;
}
