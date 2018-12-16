#pragma once

#include "monitor.h"

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>

// Enables to use the observer pattern with the callback_container class
// note: More effective than usage of std::bind
// Example usage:
//
// class wtf_observer
// {
// public:
//     virtual ~wtf_observer() = default;
//     virtual void observe(int count, const std::string& name) = 0;
// };
//
// class my_class final : public wtf_observer
// {
// public:
//     void observe(int, const std::string&) override {}
// };
//
// my_class m{};
// observer_adapter a{std::ref(m), &wtf_observer::observe};
// auto c = callback_container<decltype(a)>::create_callback_container();
// auto cToken = c -> register_callback(std::move(a));
// c->operator()(1, "unknown");

template <typename I, typename T, typename... Args>
class observer_adapter final
{
public:
    using function_t = void (T::*)(Args...);
    observer_adapter(I instance, function_t function);
    void operator()(Args... args) const;

private:
    I m_instance;
    function_t m_function;
};

// Helper trait
template <typename I, typename T, typename... Args>
constexpr auto is_observer_adaptable_v = std::is_invocable_v<typename observer_adapter<I, T, Args...>::function_t, I, Args...>;

template <typename>
class callback_token;

// type T the type of callback method (lambda, functor, std::function etc.)
// type Mtx the mutex type to be used for const method synchronization
//      exclusive mutex (i.e. std::mutex) - all calls are fully synchronized, i.e. calling const methods from fired callbacks results in a deadlock
//      rw-mutex (i.e. std::shared_mutex) - enables to call operator() from multiple threads in the same time (the operator requires reader access)
//                          still calling const methods from fired callbacks results in a deadlock (write access is required)
//      reentrant mutex (i.e. std::recursive_mutex) - enables calling const methods from fired callbacks
//                             calling unregister from the callback will grant hat the next invocation of operator() will not trigger the unregistered callback
//                             the callback can still be triggered in the current operator() execution
template <typename T, typename Mtx = std::mutex>
class callback_container final : public std::enable_shared_from_this<callback_container<T, Mtx>>
{
public:
    using callback_t = T;
    using mutex_t = Mtx;
    using token_t = callback_token<callback_container<callback_t, mutex_t>>;
    using key_t = typename token_t::key_t;

    friend token_t;

private:
    callback_container() = default;
    // copy does not make sense
    callback_container(const callback_container&) = delete;

public:
    callback_container(callback_container&&) = default;
    callback_container& operator=(callback_container&&) = default;

    static std::shared_ptr<callback_container> create_callback_container();

    // thread safe
    // the new registered callback will be fired with the next call to operator()
    // see Mtx template argument description
    token_t register_callback(callback_t&& callback) const;

    // thread safe (not including the callback body, this must be synchronized extra if it does access shared resources)
    // note: there is no particular order of callbacks execution, i.e. do NOT relay on fact that callbacks will be executed in registration order
    template <typename... Args, typename = std::enable_if_t<std::is_invocable_v<T, Args...>>>
    void operator()(Args&&... args) const;

private:
    using callback_container_t = std::unordered_map<std::size_t, callback_t>;

    // thread safe
    // callback will be unregistered and the next call to operator() will not trigger it
    // see Mtx template argument description
    void unregister_callback(std::size_t idx) const;

    monitor<callback_container_t, mutex_t> m_callbacks;
};

// T type of callback container that will use this class as token
template <typename T>
class callback_token final
{
    friend T;

private:
    using instance_t = std::weak_ptr<const T>;
    using key_t = std::size_t;

    callback_token(const instance_t& instance, key_t idx);
    // If you need to share-own, move the instance to a std::shared_ptr
    callback_token(const callback_token&) = delete;

public:
    callback_token() = default;
    callback_token(callback_token&&) = default;
    ~callback_token();

    callback_token& operator=(callback_token&&) = default;

    void unregister();

private:
    instance_t m_instance;
    key_t m_idx;
};

template <typename I, typename T, typename... Args>
observer_adapter<I, T, Args...>::observer_adapter(I instance, function_t function)
    : m_instance{instance}
    , m_function{function}
{
    if constexpr (std::is_pointer_v<I>)
    {
        assert(m_instance != nullptr);
    }
}

template <typename I, typename T, typename... Args>
void observer_adapter<I, T, Args...>::operator()(Args... args) const
{
    std::invoke(m_function, m_instance, args...);
}

template <typename T, typename Mtx>
std::shared_ptr<callback_container<T, Mtx>> callback_container<T, Mtx>::create_callback_container()
{
    return std::shared_ptr<callback_container>(new callback_container);
}

template <typename T, typename Mtx>
typename callback_container<T, Mtx>::token_t callback_container<T, Mtx>::register_callback(callback_t&& callback) const
{
    return m_callbacks([this, callback{std::move(callback)}](callback_container_t& container) mutable {
        // TODO suboptimal
        key_t key = container.size();
        while (container.find(key) != container.end()) {
            key++;
        }
        container.emplace(key, std::move(callback));
    
        return token_t(this->shared_from_this(), key);
    });
}

template <typename T, typename Mtx>
template <typename... Args, typename>
void callback_container<T, Mtx>::operator()(Args&&... args) const
{
    m_callbacks([&args...](const callback_container_t& container) {
        // TODO is necessary only if the mutex is reentrant, i.e. std::recursive_mutex
        // copy will ensure, that the container can be safely modified from the callback
        // without invalidating the iterators used in the following for loop
        const auto containerCopy = container;

        for (const auto& callback : containerCopy)
        {
            std::invoke(callback.second, std::forward<Args>(args)...);
        }
    });
}

template <typename T, typename Mtx>
void callback_container<T, Mtx>::unregister_callback(std::size_t idx) const
{
    return m_callbacks([=](callback_container_t& container) {
        const auto removedCount = container.erase(idx);
        assert(removedCount == 1);
    });
}

template <typename T>
callback_token<T>::callback_token(const instance_t& instance, key_t idx)
    : m_instance(instance)
    , m_idx{idx}
{
}

template <typename T>
callback_token<T>::~callback_token()
{
    unregister();
}

template <typename T>
void callback_token<T>::unregister()
{
    auto instanceLocked = m_instance.lock();
    if (instanceLocked)
    {
        instanceLocked->unregister_callback(m_idx);
    }
    m_instance.reset();
}