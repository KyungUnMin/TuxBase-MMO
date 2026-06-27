#pragma once

template <typename T, UINT32 Size = 0>
class LockQueue
{
public:
    LockQueue();
    ~LockQueue() = default;

    LockQueue(const LockQueue&) = delete;
    LockQueue(LockQueue&&) = delete;
    LockQueue& operator=(const LockQueue&) = delete;
    LockQueue& operator=(LockQueue&&) = delete;

public:
    void Push(const T& data);
    void Push(T&& data);

    template <typename... Args>
    void Emplace(Args&&... args);

    T Pop();
    bool TryPop(T& data);

    UINT32 GetSize() const;
    bool IsEmpty() const;

private:
    UINT32 m_headIndex;
    UINT32 m_tailIndex;
    UINT32 m_count;
    mutable std::mutex m_mutex;
    std::array<T, Size> m_queue;
};

template <typename T>
class LockQueue<T, 0>
{
public:
    LockQueue() = default;
    ~LockQueue() = default;

    LockQueue(const LockQueue&) = delete;
    LockQueue(LockQueue&&) = delete;
    LockQueue& operator=(const LockQueue&) = delete;
    LockQueue& operator=(LockQueue&&) = delete;

public:
    void Push(const T& data);
    void Push(T&& data);

    template <typename... Args>
    void Emplace(Args&&... args);

    T Pop();
    bool TryPop(T& data);

    UINT32 GetSize() const;
    bool IsEmpty() const;

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
};

#include "LockQueue.inl"
