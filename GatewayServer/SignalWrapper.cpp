
#include <stdio.h>
#include <signal.h>

#include "SignalWrapper.hpp"

Function_SignalHandler CSignalWrapper::m_pfUSR1 = NULL;
Function_SignalHandler CSignalWrapper::m_pfUSR2 = NULL;
int CSignalWrapper::m_iSignalArgUSR1 = 0;
int CSignalWrapper::m_iSignalArgUSR2 = 0;

void CSignalWrapper::IgnoreSignalSet()
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

void CSignalWrapper::SetHandler_USR1(Function_SignalHandler pfUSR1, int iSignalArg)
{
	CSignalWrapper::m_pfUSR1 = pfUSR1;
	CSignalWrapper::m_iSignalArgUSR1 = iSignalArg;
	RegisterSignal_USR1();
}

void CSignalWrapper::SetHandler_USR2(Function_SignalHandler pfUSR2, int iSignalArg)
{
	CSignalWrapper::m_pfUSR2 = pfUSR2;
	CSignalWrapper::m_iSignalArgUSR2 = iSignalArg;
	RegisterSignal_USR2();
}

void CSignalWrapper::RegisterSignal_USR1()
{
	signal(SIGUSR1, NotifySignal_USR1);
}

void CSignalWrapper::RegisterSignal_USR2()
{
	signal(SIGUSR2, NotifySignal_USR2);
}

void CSignalWrapper::NotifySignal_USR1(int iSignalValue)
{
	printf("NotifySignal_USR1 APPCMD_RELOAD_CONFIG\n");
	(*CSignalWrapper::m_pfUSR1)(CSignalWrapper::m_iSignalArgUSR1);
	RegisterSignal_USR1();
}

void CSignalWrapper::NotifySignal_USR2(int iSignalValue)
{
	printf("NotifySignal_USR2 APPCMD_STOP_SERVICE\n");
	(*CSignalWrapper::m_pfUSR2)(CSignalWrapper::m_iSignalArgUSR2);
	RegisterSignal_USR2();
}

void CSignalWrapper::IgnoreSignal(int iSignalValue)
{
	struct sigaction sig;
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);
	sigaction(iSignalValue, &sig, NULL);
}
