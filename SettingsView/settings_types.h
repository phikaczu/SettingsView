#pragma once

#include <rapidjson/document.h>

#include <string>

namespace settings {

    namespace internal {
        template <typename T>
        T parse(const rapidjson::Document& settings, const char* memberName);
    }

    struct name
    {
        using value_type = std::string;
        static value_type parse(const rapidjson::Document& settings);
    };

    struct age
    {
        using value_type = int;
        static value_type parse(const rapidjson::Document& settings);
    };

    struct salary
    {
        using value_type = int;
        static value_type parse(const rapidjson::Document& settings);
    };

    // Or even more simply
    //struct salary : public setting_type<int, "salary"> {};
}