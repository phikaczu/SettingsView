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

// I hate macros but this is super helpful
// taken from https://stackoverflow.com/a/9544792
//
template <typename>
struct has_method_helper
{
    using type = void;
};

#define HAS_METHOD(methodname)                                                                                                        \
    template <typename T, typename = void>                                                                                            \
    struct has_##methodname##_method : std::false_type                                                                                \
    {                                                                                                                                 \
    };                                                                                                                                \
                                                                                                                                      \
    template <typename T>                                                                                                             \
    struct has_##methodname##_method<T, typename has_method_helper<decltype(std::declval<T&>().methodname())>::type> : std::true_type \
    {                                                                                                                                 \
    };                                                                                                                                \
    template <typename T>                                                                                                             \
    constexpr auto has_##methodname##_method_v = has_##methodname##_method<T>::value;