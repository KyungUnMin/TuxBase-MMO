#pragma once
#include "LockStack.h"

// ============================================================================
// LockStack<T, Size>
// ============================================================================

template <typename T, UINT32 Size>
inline LockStack<T, Size>::LockStack()
    : m_topIndex(0)
{
}

template <typename T, UINT32 Size>
template <typename... Args>
inline LockStack<T, Size>::LockStack(const Args&... args)
    : m_topIndex(Size)
{
    static_assert(0 < Size, "LockStack size must be greater than 0");
    for (UINT32 i = 0; i < Size; ++i)
        m_stack[i] = T(args...);
}

template <typename T, UINT32 Size>
inline void LockStack<T, Size>::Push(const T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(m_topIndex < Size, "Stack overflow. Size(%d) < topIndex(%d)", Size, m_topIndex);
    m_stack[m_topIndex++] = data;
}

template <typename T, UINT32 Size>
inline void LockStack<T, Size>::Push(T&& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(m_topIndex < Size, "Stack overflow. Size(%d) < topIndex(%d)", Size, m_topIndex);
    m_stack[m_topIndex++] = std::move(data);
}

template <typename T, UINT32 Size>
template <typename... Args>
inline void LockStack<T, Size>::Emplace(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(m_topIndex < Size, "Stack overflow. Size(%d) < topIndex(%d)", Size, m_topIndex);
    m_stack[m_topIndex++] = T(std::forward<Args>(args)...);
}

template <typename T, UINT32 Size>
inline T LockStack<T, Size>::Pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(0 < m_topIndex, "Stack is empty");
    return std::move(m_stack[--m_topIndex]);
}

template <typename T, UINT32 Size>
inline bool LockStack<T, Size>::TryPop(T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (0 == m_topIndex)
        return false;

    data = std::move(m_stack[--m_topIndex]);
    return true;
}

template <typename T, UINT32 Size>
inline UINT32 LockStack<T, Size>::GetSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_topIndex;
}

template <typename T, UINT32 Size>
inline bool LockStack<T, Size>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return 0 == m_topIndex;
}

// ============================================================================
// LockStack<T, 0>
// ============================================================================

template <typename T>
template <typename... Args>
inline LockStack<T, 0>::LockStack(UINT32 initSize, const Args&... args)
{
    m_stack.reserve(initSize);
    for (UINT32 i = 0; i < initSize; ++i)
        m_stack.emplace_back(args...);
}

template <typename T>
inline void LockStack<T, 0>::Reserve(UINT32 size)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stack.reserve(size);
}

template <typename T>
inline void LockStack<T, 0>::Push(const T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stack.push_back(data);
}

template <typename T>
inline void LockStack<T, 0>::Push(T&& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stack.push_back(std::move(data));
}

template <typename T>
template <typename... Args>
inline void LockStack<T, 0>::Emplace(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 나중에 또 까먹을까봐 적음.
    // std::forward : 왼값이면 왼값으로 전달(복사), 오른값이면 오른값으로 전달(이동)
    m_stack.emplace_back(std::forward<Args>(args)...);
}

template <typename T>
inline T LockStack<T, 0>::Pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    ASSERT(!m_stack.empty(), "Stack is empty");
    T data = std::move(m_stack.back());
    m_stack.pop_back();
    return data;
}

template <typename T>
inline bool LockStack<T, 0>::TryPop(T& data)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_stack.empty())
        return false;
    data = std::move(m_stack.back());
    m_stack.pop_back();
    return true;
}

template <typename T>
inline UINT32 LockStack<T, 0>::GetSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<UINT32>(m_stack.size());
}

template <typename T>
inline bool LockStack<T, 0>::IsEmpty() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stack.empty();
}
