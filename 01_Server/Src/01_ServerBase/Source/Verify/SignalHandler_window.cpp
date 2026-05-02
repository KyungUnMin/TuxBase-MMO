#include "Verify/SignalHandler.h"
#include <csignal>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::atomic<bool> SignalHandler::m_shutdownRequested = true;

/* ==================================================================
윈도우쪽은 나중에 생각해보자
================================================================== */

namespace
{
    BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType)
    {
        switch (ctrlType)
        {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            // 윈도우 콘솔 이벤트를 C 표준 SIGINT로 변환하여 동일하게 처리
            raise(SIGINT);
            return TRUE;
        default:
            return FALSE;
        }
    }
} // namespace


void SignalHandler::InitSignal()
{
    signal(SIGINT, OnSIGINT);
    signal(SIGTERM, OnSIGTERM);
    signal(SIGABRT, OnSIGABRT);
    signal(SIGSEGV, OnSIGSEGV);

    // 윈도우 전용 콘솔 이벤트 콜백 등록
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

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
    // TODO : 로그 남기고 bt 남기고 덤프 남기고
    _exit(1);
}
