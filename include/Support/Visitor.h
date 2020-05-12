#pragma once

namespace visitor {

// Visitor template declaration
template<typename... Types>
class VisitorType;

// specialization for single type
template<typename T>
class VisitorType<T> {
public:
    virtual void Visit(T&& visitable) = 0;
};

// specialization for multiple types
template<typename T, typename... Types>
class VisitorType<T, Types...> : public VisitorType<Types...> {
public:
    // promote the function(s) from the base class
    using VisitorType<Types...>::Visit;

    virtual void Visit(T&& visitable) = 0;
};

template<typename... Types>
class VisitableType {
public:
    virtual ~VisitableType() = default;
    virtual void Accept(VisitorType<Types...>& visitor) = 0;
    virtual void Accept(VisitorType<Types...>& visitor) const = 0;
};

template<typename Derived, typename... Types>
class VisitableImplType : public VisitableType<Types...> {
public:
    virtual ~VisitableImplType() = default;

    void Accept(VisitorType<Types...>& visitor) override {
        visitor.Visit(static_cast<Derived&&>(*this));
    }

    void Accept(VisitorType<Types...>& visitor) const override {
        visitor.Visit(static_cast<const Derived&&>(*this)); // TODO: why does static cast to val not work?
    }
};

template<typename ... T>
struct VisitorTypes {
    using Visitor = VisitorType<T...>;

    template<typename Derived>
    using VisitableImpl = VisitableImplType<Derived, T...>;

    using Visitable = VisitableType<T...>;
};

}