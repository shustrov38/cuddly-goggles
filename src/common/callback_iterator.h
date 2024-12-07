#pragma once

#include <functional>

namespace utils {
template <typename T>
class CallbackIterator {
    class TypeWrapper {
        using CallbackT = std::function<void(T const&)>;

    public:
        TypeWrapper& operator=(T const& other) {
            mCallback(other);
            return *this;
        }

        void SetCallback(CallbackT func)
        {
            mCallback = func;
        }

    private:
        static void DefaultCallback(T const&)
        {
        }

        CallbackT mCallback {DefaultCallback};
    };

public:
    using iterator_category = std::output_iterator_tag;

    TypeWrapper &operator*() {  return mVal; }
    TypeWrapper *operator->() { return &mVal; }

    CallbackIterator() = default;

    template <typename Func>
    CallbackIterator(Func func)
    {
        mVal.SetCallback(func);
    }

    CallbackIterator& operator ++()
    {
        return *this;
    }

    CallbackIterator& operator ++(int)
    {
        return *this;
    }

private:
    TypeWrapper mVal;
};
} // namespace utils