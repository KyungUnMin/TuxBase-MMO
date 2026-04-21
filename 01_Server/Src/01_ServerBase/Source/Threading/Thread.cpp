#include "Threading/Thread.h"
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

Thread::Thread()
{
}

Thread::~Thread()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void Thread::Start(std::string_view name, std::function<void()> workerThread)
{
    ASSERT(!name.empty(), "Name is empty");
    ASSERT(workerThread, "WorkerThread is null");
    ASSERT(!m_thread.joinable(), "Thread is already running");
    m_thread = std::thread([this, name = std::string(name), workerThread = std::move(workerThread)]()
    {
        SetName(name);
        workerThread();
    });
}

void Thread::SetName(std::string_view name)
{
#ifdef _WIN32
    wchar_t wideName[64] = {};
    MultiByteToWideChar(CP_UTF8, 0, name.data(), name.size(), wideName, 64);
    SetThreadDescription(m_thread.native_handle(), wideName);
#else
    char clampedName[16]; // 15자 제한
    std::strncpy(clampedName, name.data(), 15);
    clampedName[15] = '\0';
    pthread_setname_np(m_thread.native_handle(), clampedName);
#endif
}
