#pragma once

class SignalHandler
{
public:
    static void InitSignal();
    static bool IsShutdownRequested() { return m_shutdownRequested.load(); }

private:
    SignalHandler() = delete;
    ~SignalHandler() = delete;
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler& operator=(SignalHandler&&) = delete;

    static void OnSIGINT(int signum);  // Ctrl+C 인터럽트
    static void OnSIGTERM(int signum); // 정상 종료 요청 (kill 기본값, Docker stop)
    static void OnSIGABRT(int signum); // abort() 호출 시 (크래시 덤프 수집)
    static void OnSIGSEGV(int signum); // 잘못된 메모리 접근 (크래시 덤프 수집)

    // 나중에 핫 리로드 기능을 만들게 된다면 그때 쓰자(리눅스만 존재, 터미널 연결 끊김 or 설정 리로드)
    // static void OnSIGHUP(int signum);

private:
    static std::atomic<bool> m_shutdownRequested;
};
