#include "Verify/SignalHandler.h"
#include <csignal>

std::atomic<bool> SignalHandler::m_shutdownRequested = true;

void SignalHandler::InitSignal()
{
    signal(SIGINT, OnSIGINT);
    signal(SIGTERM, OnSIGTERM);
    signal(SIGABRT, OnSIGABRT);
    signal(SIGSEGV, OnSIGSEGV);
    // signal(SIGHUP, OnSIGHUP); // 나중에 핫 리로드 시 사용
    signal(SIGPIPE, SIG_IGN); // 끊긴 소켓에 쓰기 시도 시 발생. 무시
    m_shutdownRequested.store(false);
}

void SignalHandler::OnSIGINT(int signum)
{
    // TODO : 로그 남기기

    m_shutdownRequested.store(true);
}

void SignalHandler::OnSIGTERM(int signum)
{
    // TODO : 로그 남기기

    m_shutdownRequested.store(true);
}

void SignalHandler::OnSIGABRT(int signum)
{
    // TODO : 로그 남기고 bt 남기고 덤프는 알아서 남겨질테고.
    _exit(1);
}

void SignalHandler::OnSIGSEGV(int signum)
{
    // TODO : 로그 남기고 bt 남기고 덤프는 알아서 남겨질테고.
    _exit(1);
}