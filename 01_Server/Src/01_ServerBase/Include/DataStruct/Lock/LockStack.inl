#pragma once
#include "LockStack.h"

template <typename T>
template <typename... Args>
inline LockStack<T>::LockStack(UINT32 initSize, const Args&... args)
{
    m_stack.reserve(initSize);
    for (UINT32 i = 0; i < initSize; ++i)
        m_stack.emplace_back(args...);
}

template <typename T>
inline void LockStack<T>::Reserve(UINT32 size)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stack.reserve(size);
}

template <typename T>
inline void LockStack<T>::Push(const T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stack.push_back(data);
}

template <typename T>
inline void LockStack<T>::Push(T&& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stack.push_back(std::move(data));
}

template <typename T>
template <typename... Args>
inline void LockStack<T>::Emplace(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 나중에 또 까먹을까봐 적음.
    // std::forward : 왼값이면 왼값으로 전달(복사), 오른값이면 오른값으로 전달(이동)
    m_stack.emplace_back(std::forward<Args>(args)...);
}

template <typename T>
inline T LockStack<T>::Pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(!m_stack.empty(), "Stack is empty");
    T data = std::move(m_stack.back());
    m_stack.pop_back();
    return data;
}

template <typename T>
inline bool LockStack<T>::TryPop(T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_stack.empty())
        return false;
    data = std::move(m_stack.back());
    m_stack.pop_back();
    return true;
}

template <typename T>
inline UINT32 LockStack<T>::GetSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<UINT32>(m_stack.size());
}

template <typename T>
inline bool LockStack<T>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stack.empty();
}