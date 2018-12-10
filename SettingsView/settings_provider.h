#pragma once

#include "settings_view.h"
#include <rapidjson/document.h>

#include <memory>
#include <string>

class settings_provider
{
public:
    explicit settings_provider(rapidjson::Document&& settings);

    template <typename ...Args>
    settings_view<Args...> get_view(const std::string& consumerName);

private:
    rapidjson::Document m_settings;
};

template <typename... Args>
inline settings_view<Args...> settings_provider::get_view(const std::string& consumerName)
{
    return settings_view<Args...>(Args::parse(m_settings)...);
}
