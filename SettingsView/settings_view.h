#pragma once

#include "utils.h"
#include <tuple>

template <typename... Args>
class settings_view
{
public:
    settings_view(typename Args::value_type&& ... values);

    //! Returns the value of the setting type \c T
    //! \tparam T setting type, must be part of the class argument pack \c Args
    template <typename T, typename = std::enable_if_t<is_any_of<T, Args...>>>
    const typename T::value_type& get() const noexcept;

private:
    std::tuple<typename Args::value_type...> m_values;
};

template <typename... Args>
settings_view<Args...>::settings_view(typename Args::value_type&& ... values)
    : m_values(values ...)
{
}

template <typename... Args>
template <typename T, typename>
const typename T::value_type& settings_view<Args...>::get() const noexcept
{
    return std::get<pack_index_v<T,Args...>>(m_values);
}
