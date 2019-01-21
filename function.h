#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <utility>
#include <variant>
#include <memory>

static const size_t B_SIZE = 64;

enum OBJECT_TYPE {
    EMPTY = 0,
    SMALL,
    BIG
};

template<typename T>
class function;

template<typename Ret, typename ...Args>
class function<Ret(Args...)> {
public:
    Ret operator()(Args...args) const {
        switch (type) {
            case EMPTY:
                throw std::bad_function_call();
            case SMALL:
                return ((function_holder_base *) buf)->invoke(std::forward<Args>(args)...);
            case BIG:
                return holder->invoke(std::forward<Args>(args)...);
        }
    }

    function() noexcept : holder(nullptr), type(EMPTY) {}

    function(std::nullptr_t) noexcept : holder(nullptr), type(EMPTY) {}

    function(const function &other) {
        function_holder_base *c = (function_holder_base *) other.buf;
        type = other.type;
        switch (type) {
            case EMPTY:
                holder = nullptr;
                break;
            case SMALL:
                c->create_small_copy(buf);
                break;
            case BIG:
                holder = other.holder->copy();
                break;
        }

    }

    function(function &&other) noexcept : holder(nullptr) {
        std::swap(buf, other.buf);
        std::swap(type, other.type);
        other.type = EMPTY;
    }

    function &operator=(const function &other) {
        function tmp(other);
        swap(tmp);
        return *this;
    }

    function &operator=(function &&other) noexcept {
        auto tmp(std::forward<function>(other));
        swap(tmp);
        return *this;
    }

    void swap(function &other) noexcept {
        std::swap(buf, other.buf);
        std::swap(other.type, type);
    }

    explicit operator bool() const noexcept {
        return type != EMPTY;
    }


    template<typename T>
    function(T t) {
        if constexpr (sizeof(function_holder<T>(t)) <= B_SIZE) {
            type = SMALL;
            new(buf) function_holder<T>(std::move(t));
        } else {
            type = BIG;
            new(buf) std::unique_ptr<function_holder < T>>
            (std::make_unique<function_holder < T>>
            (std::move(t)));
        }
    }

    ~function() {
        if (type == SMALL) {
            ((function_holder_base *) buf)->~function_holder_base();
        } else {
            holder.reset();
        }
    }


    class function_holder_base {
    public:
        function_holder_base() {};

        virtual ~function_holder_base() {};

        virtual Ret invoke(Args...) = 0;

        virtual std::unique_ptr<function_holder_base> copy() const = 0;

        virtual void create_small_copy(void *adr) = 0;

    };

    template<typename Func>
    class function_holder : public function_holder_base {
    public:
        function_holder(const Func &func) : f(func) {}

        ~function_holder() override = default;

        Ret invoke(Args...args) {
            return f(std::forward<Args>(args)...);
        }

        void create_small_copy(void *adr) {
            new(adr) function_holder<Func>(f);

        }

        std::unique_ptr<function_holder_base> copy() const override {
            return std::make_unique<function_holder<Func>>(f);
        }

        Func f;

    };

private:
    union {
        std::unique_ptr<function_holder_base> holder;
        char buf[B_SIZE];
    };
    OBJECT_TYPE type;

};

#endif /