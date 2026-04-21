#pragma once

class Thread
{
public:
    Thread();
    ~Thread();

    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&&) = delete;

    void Start(std::string_view name, std::function<void()> workerThread);

private:
    void SetName(std::string_view name);

private:
    std::thread m_thread;
};