#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <utility>
#include <variant>
#include <memory>

constexpr const size_t MAX_SIZE = 16;

template <typename T>
class function;

template <typename Ret, typename ...Args>
class function<Ret (Args...)> {
public:
    Ret operator()(Args...args) {
        return holder->invoke(args...);
    }

    function() noexcept : holder(nullptr) {}

    function(std::nullptr_t) noexcept : holder(nullptr) {}

    function(const function &other) : holder(std::move(other.holder->copy())) {
    }

    function(function &&other) noexcept : holder(std::move(other.holder)) {
    }

    function &operator=(const function &other) {
        holder(std::move(other.holder->copy()));
        return *this;
    }

    function &operator=(function &&other) noexcept {
        holder(std::move(other.holder));
        return *this;
    }

    void swap(function& other) noexcept {
        std::swap(holder, other.holder);
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(holder);
    }

    template<typename T>
    function &operator=(const T &t) {
        holder = std::make_unique<function_holder<T>>(t);
        return *this;
    }

    template<typename T>
    function(T t) {
        holder = std::make_unique<function_holder<T>>(std::move(t));
    }


    class function_holder_base {
    public:
        function_holder_base() {};
        virtual ~function_holder_base() {};
        virtual Ret invoke(Args...) = 0;
        virtual std::unique_ptr<function_holder_base> copy() const = 0;
    };

    template <typename Func>
    class function_holder : public function_holder_base {
    public:
        function_holder(const Func& func) : f(func) {}

        ~function_holder() = default;

         Ret invoke(Args...args){
            return f(args...);
        }

        std::unique_ptr<function_holder_base> copy() const override {
            return std::make_unique<function_holder<Func>>(f);
        }

        Func f;

    };

private:

    std::unique_ptr<function_holder_base> holder;
};


#endif /