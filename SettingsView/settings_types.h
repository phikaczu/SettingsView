#pragma once

#include "salary_level.h"

#include <string>
#include <type_traits>

namespace settings {

namespace internal {

    // The string literal could be passed as a template parameter
    // but I thing it is more readable when it is not made.
    template <typename In, typename Out = In>
    struct types
    {
        using source_type = In;
        using value_type = Out;

        // pure static class
        types() = delete;

        static value_type parse(source_type&& input)
        {
            return value_type(std::move(input));
        }
    };

}  // namespace internal

    struct name : internal::types<std::string>
    {
        static constexpr auto path = "name";
    };

    struct age : internal::types<int>
    {
        static constexpr auto path = "age";
    };

    struct salary : internal::types<std::underlying_type_t<salary_level>, salary_level>
    {
        static constexpr auto path = "salary";
    };
}