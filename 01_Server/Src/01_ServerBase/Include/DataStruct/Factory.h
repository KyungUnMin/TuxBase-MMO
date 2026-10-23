#pragma once

#include <tuple>
template <typename Base>
class IFactory
{
public:
    IFactory() = default;
    virtual ~IFactory() = default;
    IFactory(const IFactory&) = delete;
    IFactory(IFactory&&) = delete;
    IFactory& operator=(const IFactory&) = delete;
    IFactory& operator=(IFactory&&) = delete;

    virtual SPtr<Base> Create() = 0;
};

template <typename Base, typename Derived, typename... Args>
class Factory : public IFactory<Base>
{
public:
    explicit Factory(Args... args)
        : m_args(std::forward<Args>(args)...)
    {
    }
    ~Factory() override = default;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;

    SPtr<Base> Create() override
    {
        return std::apply([](Args... args)
        {
            return std::make_shared<Derived>(args...);
        }, m_args);
    }

private:
    std::tuple<Args...> m_args;
};