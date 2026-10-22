#pragma once

template <typename Signature = void()>
class Delegate;

template <typename... Args>
class Delegate<void(Args...)>
{
public:
    using Callback = std::function<void(Args...)>;
    using EventID = UINT32;

public:
    Delegate() = default;
    ~Delegate() = default;
    Delegate(const Delegate&) = delete;
    Delegate(Delegate&&) = delete;
    Delegate& operator=(const Delegate&) = delete;
    Delegate& operator=(Delegate&&) = delete;

    EventID Subscribe(Callback&& callback)
    {
        const EventID eventID = static_cast<EventID>(m_callbacks.size());
        m_callbacks.push_back(std::move(callback));
        return eventID;
    }

    // 삭제는 필요해지면 그때 만들자
    // void Unsubscribe(EventID eventID)
    // {
    //     std::swap()
    // }

    void Invoke(Args... args)
    {
        for (const Callback& callback : m_callbacks)
        {
            callback(args...);
        }
    }

    UINT32 Count() const
    {
        return static_cast<UINT32>(m_callbacks.size());
    }

    void Clear()
    {
        m_callbacks.clear();
    }

private:
    std::vector<Callback> m_callbacks;
};
