#pragma once

#include <tuple>


template <typename... Ts>
struct CreateArgs
{
};

template <typename Base, typename... CreateArgTypes>
class IFactory
{
public:
    IFactory() = default;
    virtual ~IFactory() = default;
    IFactory(const IFactory&) = delete;
    IFactory(IFactory&&) = delete;
    IFactory& operator=(const IFactory&) = delete;
    IFactory& operator=(IFactory&&) = delete;

    virtual SPtr<Base> Create(CreateArgTypes... createArgs) = 0;
};


template <typename Base, typename Derived, typename CreateArgList, typename... FixedArgs>
class Factory;

template <typename Base, typename Derived, typename... CreateArgTypes, typename... FixedArgs>
class Factory<Base, Derived, CreateArgs<CreateArgTypes...>, FixedArgs...> : public IFactory<Base, CreateArgTypes...>
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

    SPtr<Base> Create(CreateArgTypes... createArgs) override
    {
        return std::apply([&createArgs...](FixedArgs... fixedArgs)
        {
            return std::make_shared<Derived>(fixedArgs..., createArgs...);
        }, m_fixedArgs);
    }

private:
    std::tuple<FixedArgs...> m_fixedArgs;
};
