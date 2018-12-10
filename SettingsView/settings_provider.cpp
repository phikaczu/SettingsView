#include "pch.h"

#include "settings_provider.h"

#include <rapidjson/error/en.h>

settings_provider::settings_provider(rapidjson::Document&& settings)
    : m_settings{ std::move(settings) }
{
    if (m_settings.HasParseError()) {
        const auto errorMsg = rapidjson::GetParseError_En(m_settings.GetParseError());
        throw std::runtime_error(errorMsg);
    }
}
