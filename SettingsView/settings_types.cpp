#include "pch.h"

#include "settings_types.h"

namespace settings {

namespace internal {

    template<>
    std::string parse<std::string>(const rapidjson::Document& settings, const char* memberName)
    {
        const auto valueIt = settings.FindMember(memberName);
        if (valueIt == settings.MemberEnd())
        {
            throw std::runtime_error(std::string("Member '") + memberName + "' not found");
        }

        const auto& value = valueIt->value;
        if (!value.IsString())
        {
            throw std::runtime_error(std::string("Member '") + memberName + "' is not of type string");
        }

        return std::string(value.GetString(), value.GetStringLength());
    }

    template <>
    int parse<int>(const rapidjson::Document& settings, const char* memberName)
    {
        const auto valueIt = settings.FindMember(memberName);
        if (valueIt == settings.MemberEnd())
        {
            throw std::runtime_error(std::string("Member '") + memberName + "' not found");
        }

        const auto& value = valueIt->value;
        if (!value.IsInt())
        {
            throw std::runtime_error(std::string("Member '") + memberName + "' is not of type int");
        }

        return value.GetInt();
    }

    // Explicit instantiation
    template std::string parse<std::string>(const rapidjson::Document&, const char*);
    template int parse<int>(const rapidjson::Document&, const char*);
}

name::value_type name::parse(const rapidjson::Document& settings)
{
    return internal::parse<std::string>(settings, "name");
}

age::value_type age::parse(const rapidjson::Document& settings)
{
    return internal::parse<int>(settings, "age");
}

salary::value_type salary::parse(const rapidjson::Document& settings)
{
    return internal::parse<int>(settings, "salary");
}

}  // namespace settings