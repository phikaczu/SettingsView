#pragma once

#include <cstddef>
#include <type_traits>

template <class T, class... Ts>
constexpr auto is_any_of = std::disjunction_v<std::is_same<T, Ts>...>;

template <typename T, typename... Ts>
struct pack_index;

template <typename T, typename... Ts>
struct pack_index<T, T, Ts...> : std::integral_constant<std::size_t, 0>
{
};

template <typename T, typename U, typename... Ts>
struct pack_index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + pack_index<T, Ts...>::value>
{
};

template <typename T, typename... Ts>
constexpr auto pack_index_v = pack_index<T, Ts...>::value;