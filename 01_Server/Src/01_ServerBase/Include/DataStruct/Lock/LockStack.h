#pragma once

template <typename T>
class LockStack
{
public:
    LockStack() = default;
    template <typename... Args>
    LockStack(UINT32 initSize, const Args&... args);
    ~LockStack() = default;

    LockStack(const LockStack&) = delete;
    LockStack(LockStack&&) = delete;
    LockStack& operator=(const LockStack&) = delete;
    LockStack& operator=(LockStack&&) = delete;

public:
    void Reserve(UINT32 size);
    void Push(const T& data);
    void Push(T&& data);

    template <typename... Args>
    void Emplace(Args&&... args);

    T Pop();
    bool TryPop(T& data);

    UINT32 GetSize() const;
    bool IsEmpty() const;

private:
    std::vector<T> m_stack;
    std::mutex m_mutex;
    // TODO : 나중에 자체 락 만들어서 적용해보자 (스핀락, 슬립락, 컨텍스트락, 복합락) -> 생성시 템플릿으로 지정하게끔 하면 됨
};

#include "LockStack.inl"
