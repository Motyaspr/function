#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <utility>
#include <variant>
#include <memory>

static const size_t B_SIZE = 64;


template <typename T>
class function;

template <typename Ret, typename ...Args>
class function<Ret (Args...)> {
public:
    Ret operator()(Args...args) const {
        if (!is_small)
            return holder->invoke(args...);
        function_holder_base * tmp= (function_holder_base *)buf;
        return tmp->invoke(args...);
    }

    function() noexcept : holder(nullptr), is_small(false) {}

    function(std::nullptr_t) noexcept : holder(nullptr), is_small(false) {}

    function(const function &other) {
        function_holder_base * tmp = (function_holder_base *)other.buf;
        is_small = other.is_small;
        if (other.is_small)
            memcpy(buf, other.buf, B_SIZE);
        else
            holder = other.holder->copy();

    }

    function(function &&other) noexcept{
        function_holder_base * tmp = (function_holder_base *)other.buf;
        is_small = other.is_small;
        if (other.is_small)
            tmp->create_small_copy(buf);
        else
            new(buf) std::unique_ptr<function_holder_base>(std::move(other.holder));
    }

    function &operator=(const function &other) {
        function tmp(other);
        swap(other);
        return *this;
    }

    function &operator=(function &&other) noexcept {
        function_holder_base * tmp = (function_holder_base *)other.buf;
        if (is_small)
            ((function_holder_base *) other.buf)->~function_holder_base();
        else
            holder.reset();

        is_small = other.is_small;
        if (other.is_small)
            tmp->create_small_copy(buf);
        else
            new(buf) std::unique_ptr<function_holder_base>(std::move(other.holder));
        return *this;
    }

    void swap(function& other) noexcept {
        std::swap(buf, other.buf);
        std::swap(is_small, other.is_small);
    }

    explicit operator bool() const noexcept {
        if (is_small)
            return true;
        return static_cast<bool>(holder);
    }

    template<typename T>
    function &operator=(T t) {
        if (sizeof(T) < B_SIZE){
            is_small = true;
            new(buf) function_holder<T>(t);
        }
        else{
            is_small = false;
            holder = std::make_unique<function_holder<T>>(std::move(t));
        }
        return *this;
    }

    template<typename T>
    function(T t) {
        if (sizeof(T) < B_SIZE){
            is_small = true;
            new(buf) function_holder<T>(t);
        }
        else{
            is_small = false;
            holder = std::make_unique<function_holder<T>>(std::move(t));
        }
    }

    ~function() {
        if (is_small) {
            function_holder_base *m = (function_holder_base *) buf;
            m->~function_holder_base();
        } else {
            holder.reset();
        }
    }



    class function_holder_base {
    public:
        function_holder_base() {};
        virtual ~function_holder_base() {};
        virtual Ret invoke(Args...) = 0;
        virtual void create_small_copy(void *pos) = 0;

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

        void create_small_copy(void *adr) override {
            new(adr) function_holder<Func>(f);
        }

        /*void create_big_copy(void *adr) override {
            new(adr) std::unique_ptr<function_holder<Func>>(new function_holder<Func>(f));
        }*/

        Func f;

    };

private:
union {
    std::unique_ptr<function_holder_base> holder;
    char buf[B_SIZE];
};
    bool is_small;
};

#endif /