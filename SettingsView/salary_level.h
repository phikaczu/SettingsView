#pragma once

#include <ostream>
#include <type_traits>

enum class salary_level
{
    unknown = 0,
    low = 1,
    average = 2,
    high = 3
};

std::ostream& operator<<(std::ostream& os, salary_level salary)
{
    switch (salary)
    {
    case salary_level::unknown: os << "unknown";    break;
    case salary_level::low:     os << "low";        break;
    case salary_level::average: os << "average";    break;
    case salary_level::high:    os << "high";       break;

    default: os << static_cast<std::underlying_type_t<salary_level>>(salary); break;
    }

    return os;
}