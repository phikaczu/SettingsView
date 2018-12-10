#include "pch.h"

#include "json_settings_reader.h"

#include <rapidjson/error/en.h>

json_settings_reader::json_settings_reader(rapidjson::Document&& settings)
    : m_settings(std::move(settings))
{
    if (m_settings.HasParseError())
    {
        const auto errorMsg = rapidjson::GetParseError_En(m_settings.GetParseError());
        throw std::runtime_error(errorMsg);
    }
}

void json_settings_reader::get(int& value, const std::string& path)
{
    get(value, path.c_str());
}

void json_settings_reader::get(int& value, const char* path)
{
    const auto valueIt = m_settings.FindMember(path);
    if (valueIt == m_settings.MemberEnd())
    {
        throw std::runtime_error(std::string("Member '") + path + "' not found");
    }

    const auto& jsonValue = valueIt->value;
    if (!jsonValue.IsInt())
    {
        throw std::runtime_error(std::string("Member '") + path + "' is not of type int");
    }

    value = jsonValue.GetInt();
}

void json_settings_reader::get(std::string& value, const std::string& path)
{
    get(value, path.c_str());
}

void json_settings_reader::get(std::string& value, const char* path)
{
    const auto valueIt = m_settings.FindMember(path);
    if (valueIt == m_settings.MemberEnd())
    {
        throw std::runtime_error(std::string("Member '") + path + "' not found");
    }

    const auto& jsonValue = valueIt->value;
    if (!jsonValue.IsString())
    {
        throw std::runtime_error(std::string("Member '") + path + "' is not of type string");
    }

    value = std::string(jsonValue.GetString(), jsonValue.GetStringLength());
}
