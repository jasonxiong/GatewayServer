/**
*@file SignalUtility.cpp
*@author jasonxiong
*@date 2009-12-09
*@version 1.0
*@brief 信号操作类的实现文件
*
*
*/

//在这添加准备库头文件
#include <stdio.h>
#include <signal.h>

#include "SignalUtility.hpp"

using namespace ServerLib;

Function_SignalHandler CSignalUtility::m_pfUSR1 = NULL;
Function_SignalHandler CSignalUtility::m_pfUSR2 = NULL;
Function_SignalHandler CSignalUtility::m_pfQuit = NULL;
int CSignalUtility::m_iSignalArgUSR1 = 0;
int CSignalUtility::m_iSignalArgUSR2 = 0;
int CSignalUtility::m_iSignalArgQuit = 0;


void CSignalUtility::IgnoreSignalSet()
{
    IgnoreSignal(SIGINT);
    IgnoreSignal(SIGHUP);
    IgnoreSignal(SIGQUIT);
    IgnoreSignal(SIGTTOU);
    IgnoreSignal(SIGTTIN);
    IgnoreSignal(SIGCHLD);
    IgnoreSignal(SIGTERM);
    IgnoreSignal(SIGHUP);
    IgnoreSignal(SIGPIPE);
}

void CSignalUtility::SetHandler_USR1(Function_SignalHandler pfUSR1, int iSignalArg)
{
    CSignalUtility::m_pfUSR1 = pfUSR1;
    CSignalUtility::m_iSignalArgUSR1 = iSignalArg;
    RegisterSignal_USR1();
}

void CSignalUtility::SetHandler_USR2(Function_SignalHandler pfUSR2, int iSignalArg)
{
    CSignalUtility::m_pfUSR2 = pfUSR2;
    CSignalUtility::m_iSignalArgUSR2 = iSignalArg;
    RegisterSignal_USR2();
}

void CSignalUtility::SetHandler_QUIT(Function_SignalHandler pfQuit, int iSignalArgQuit)
{
    CSignalUtility::m_pfQuit = pfQuit;
    CSignalUtility::m_iSignalArgQuit = iSignalArgQuit;
    RegisterSignal_Quit();
}

void CSignalUtility::RegisterSignal_USR1()
{
    signal(SIGUSR1, NotifySignal_USR1);
}

void CSignalUtility::RegisterSignal_USR2()
{
    signal(SIGUSR2, NotifySignal_USR2);
}

void CSignalUtility::RegisterSignal_Quit()
{
    signal(SIGQUIT, NotifySignal_Quit);
}

void CSignalUtility::NotifySignal_USR1(int iSignalValue)
{
    //printf("NotifySignal_USR1 APPCMD_RELOAD_CONFIG\n");
    (*CSignalUtility::m_pfUSR1)(CSignalUtility::m_iSignalArgUSR1);
    RegisterSignal_USR1();
}

void CSignalUtility::NotifySignal_USR2(int iSignalValue)
{
    //printf("NotifySignal_USR2 APPCMD_STOP_SERVICE\n");
    (*CSignalUtility::m_pfUSR2)(CSignalUtility::m_iSignalArgUSR2);
    RegisterSignal_USR2();
}

void CSignalUtility::NotifySignal_Quit(int iSignalValue)
{
    (*CSignalUtility::m_pfQuit)(CSignalUtility::m_iSignalArgQuit);
    RegisterSignal_Quit();
}

void CSignalUtility::IgnoreSignal(int iSignalValue)
{
    struct sigaction sig;
    sig.sa_handler = SIG_IGN;
    sig.sa_flags = 0;
    sigemptyset(&sig.sa_mask);
    sigaction(iSignalValue, &sig, NULL);
}

void CSignalUtility::SetSignalHandler(int iSignalValue, Function_SignalHandler pfHandler)
{
    signal(iSignalValue, pfHandler);
}
