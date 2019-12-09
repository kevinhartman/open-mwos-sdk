#pragma once

namespace visitor {

// Visitor template declaration
template<typename... Types>
class Visitor;

// specialization for single type
template<typename T>
class Visitor<T> {
public:
    virtual void Visit(T&& visitable) = 0;
};

// specialization for multiple types
template<typename T, typename... Types>
class Visitor<T, Types...> : public Visitor<Types...> {
public:
    // promote the function(s) from the base class
    using Visitor<Types...>::Visit;

    virtual void Visit(T&& visitable) = 0;
};

template<typename... Types>
class Visitable {
public:
    virtual ~Visitable() = default;
    virtual void Accept(Visitor<Types...>& visitor) = 0;
    virtual void Accept(Visitor<Types...>& visitor) const = 0;
};

template<typename Derived, typename... Types>
class VisitableImpl : public Visitable<Types...> {
public:
    virtual ~VisitableImpl() = default;

    void Accept(Visitor<Types...>& visitor) override {
        visitor.Visit(static_cast<Derived&&>(*this));
    }

    void Accept(Visitor<Types...>& visitor) const override {
        visitor.Visit(static_cast<const Derived&&>(*this)); // TODO: why does static cast to val not work?
    }
};

template<typename ... T>
struct VisitorTypes {
    using Visitor = Visitor<T...>;

    template<typename Derived>
    using VisitableImpl = VisitableImpl<Derived, T...>;

    using Visitable = Visitable<T...>;
};

}