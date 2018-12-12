#pragma once

#include <algorithm>
#include <functional>
#include <vector>

template <typename T>
class instance_tracker final
{
public:
    using instance_t = T;
    using container_t = std::vector<std::reference_wrapper<const instance_t>>;

    instance_tracker() = delete;

    static void register_instance(const instance_t& instance)
    {
        m_instances.emplace_back(instance);
    }

    static void unregister_instance(const instance_t& instance)
    {
        const auto it
            = std::find_if(m_instances.cbegin(), m_instances.cend(), [&](container_t::value_type item) { return &item.get() == &instance; });
        m_instances.erase(it);
    }

    static const container_t& get()
    {
        return m_instances;
    }

private:
    inline static container_t m_instances;
};

template<typename T>
class tracked
{
protected:
    tracked()
    {
        instance_tracker<T>::register_instance(static_cast<const T&>(*this));
    }

    ~tracked()
    {
        instance_tracker<T>::unregister_instance(static_cast<const T&>(*this));
    }
};
