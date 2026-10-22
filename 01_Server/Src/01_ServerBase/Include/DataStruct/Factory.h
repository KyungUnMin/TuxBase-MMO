#pragma once

template <typename Base, typename... Args>
class IFactory
{
public:
    IFactory() = default;
    virtual ~IFactory() = default;
    IFactory(const IFactory&) = delete;
    IFactory(IFactory&&) = delete;
    IFactory& operator=(const IFactory&) = delete;
    IFactory& operator=(IFactory&&) = delete;

    virtual SPtr<Base> Create(Args... args) = 0;
};

template <typename Base, typename Derived, typename... Args>
class Factory : public IFactory<Base, Args...>
{
public:
    Factory() = default;
    ~Factory() override = default;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;

    SPtr<Base> Create(Args... args) override
    {
        return std::make_shared<Derived>(std::forward<Args>(args)...);
    }
};