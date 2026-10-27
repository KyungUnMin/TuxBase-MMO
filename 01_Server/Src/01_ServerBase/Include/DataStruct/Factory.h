#pragma once

template <typename Signature>
class IFactory;

template <typename Base, typename... RuntimeArgTypes>
class IFactory<Base(RuntimeArgTypes...)>
{
public:
    IFactory() = default;
    virtual ~IFactory() = default;
    IFactory(const IFactory&) = delete;
    IFactory(IFactory&&) = delete;
    IFactory& operator=(const IFactory&) = delete;
    IFactory& operator=(IFactory&&) = delete;

    virtual SPtr<Base> Create(RuntimeArgTypes... runtimeArgs) = 0;
};


template <typename Signature, typename Derived, typename... FixedArgs>
class Factory;

template <typename Base, typename... RuntimeArgTypes, typename Derived, typename... FixedArgs>
class Factory<Base(RuntimeArgTypes...), Derived, FixedArgs...> : public IFactory<Base(RuntimeArgTypes...)>
{
public:
    explicit Factory(FixedArgs... fixedArgs)
        : m_fixedArgs(std::forward<FixedArgs>(fixedArgs)...)
    {
    }
    ~Factory() override = default;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;

    SPtr<Base> Create(RuntimeArgTypes... runtimeArgs) override
    {
        return std::apply([&runtimeArgs...](FixedArgs... fixedArgs)
        {
            return std::make_shared<Derived>(fixedArgs..., runtimeArgs...);
        }, m_fixedArgs);
    }

private:
    std::tuple<FixedArgs...> m_fixedArgs;
};
