#pragma once
#include "LockQueue.h"

// ============================================================================
// LockQueue<T, Size>
// ============================================================================

template <typename T, UINT32 Size>
inline LockQueue<T, Size>::LockQueue()
    : m_headIndex(0)
    , m_tailIndex(0)
    , m_count(0)
{
}

template <typename T, UINT32 Size>
inline void LockQueue<T, Size>::Push(const T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    CRASH(m_count < Size, "Queue overflow.");
    m_queue[m_tailIndex] = data;
    m_tailIndex = (m_tailIndex + 1) % Size;
    ++m_count;
}

template <typename T, UINT32 Size>
inline void LockQueue<T, Size>::Push(T&& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    CRASH(m_count < Size, "Queue overflow.");
    m_queue[m_tailIndex] = std::move(data);
    m_tailIndex = (m_tailIndex + 1) % Size;
    ++m_count;
}

template <typename T, UINT32 Size>
template <typename... Args>
inline void LockQueue<T, Size>::Emplace(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    CRASH(m_count < Size, "Queue overflow.");
    m_queue[m_tailIndex] = T(std::forward<Args>(args)...);
    m_tailIndex = (m_tailIndex + 1) % Size;
    ++m_count;
}

template <typename T, UINT32 Size>
inline T LockQueue<T, Size>::Pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(0 < m_count, "Queue is empty");
    T data = std::move(m_queue[m_headIndex]);
    m_headIndex = (m_headIndex + 1) % Size;
    --m_count;
    return data;
}

template <typename T, UINT32 Size>
inline bool LockQueue<T, Size>::TryPop(T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (0 == m_count)
        return false;

    data = std::move(m_queue[m_headIndex]);
    m_headIndex = (m_headIndex + 1) % Size;
    --m_count;
    return true;
}

template <typename T, UINT32 Size>
inline UINT32 LockQueue<T, Size>::GetSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_count;
}

template <typename T, UINT32 Size>
inline bool LockQueue<T, Size>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return 0 == m_count;
}

// ============================================================================
// LockQueue<T, 0>
// ============================================================================

template <typename T>
inline void LockQueue<T, 0>::Push(const T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(data);
}

template <typename T>
inline void LockQueue<T, 0>::Push(T&& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(data));
}

template <typename T>
template <typename... Args>
inline void LockQueue<T, 0>::Emplace(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.emplace(std::forward<Args>(args)...);
}

template <typename T>
inline T LockQueue<T, 0>::Pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(!m_queue.empty(), "Queue is empty");
    T data = std::move(m_queue.front());
    m_queue.pop();
    return data;
}

template <typename T>
inline bool LockQueue<T, 0>::TryPop(T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return false;

    data = std::move(m_queue.front());
    m_queue.pop();
    return true;
}

template <typename T>
inline UINT32 LockQueue<T, 0>::GetSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<UINT32>(m_queue.size());
}

template <typename T>
inline bool LockQueue<T, 0>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}
