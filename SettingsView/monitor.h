#pragma once

#include "utils.h"

#include <shared_mutex>

// based on Herb Sutter's presentation C++ Concurrency 2012 State of the Art(and Standard)

HAS_METHOD(lock_shared)

template <typename T, typename Mtx = std::shared_mutex>
class monitor final
{
public:
    using value_t = T;
    using mutex_t = Mtx;

    monitor(value_t value = value_t{});
    // It's unclear what are the expectations of the constructed object:
    // should it have only the same value as the parent object in time of copy, or should both have always the same value?
    // Store a copy of T in the function passed to monitor<T>::operator() for the former case and use e.g. shared_ptr<monitor<T>> for the latter case.
    monitor(const monitor&) = delete;
    monitor(monitor&&) = default;

    // The only thread safe method in this class ;)
    // Note that the following use cases are not thread safe (but cannot be prevented by the compiler)
    // monitor<T> x{};
    // T* y{};
    // x([&](const T& in){ y = &in; });
    // x([&](T& in){ y = &in; });
    template <typename F>
    decltype(auto) operator()(F f) const
    {
        // OPEN instead of the has_lock_shared_method_v a trait could be used
        if constexpr (std::is_invocable_v<F, const T> && has_lock_shared_method_v<mutex_t>) {
            std::shared_lock<mutex_t> lock{m_valueMtx};
            return f(m_value);
        }
        else {
            std::lock_guard<mutex_t> lock{m_valueMtx};
            return f(m_value);
        }
    }

private:
    mutable mutex_t m_valueMtx;
    mutable value_t m_value;
};

template <typename T, typename Mtx>
monitor<T, Mtx>::monitor(value_t value)
    : m_value{std::move(value)}
{
}
