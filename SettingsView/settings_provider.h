#pragma once

#include "settings_reader.h"
#include "settings_view.h"

#include <memory>
#include <string>

class settings_provider
{
public:
    explicit settings_provider(std::unique_ptr<settings_reader>&& settingsReader);

    template <typename... Args>
    settings_view<Args...> get_view(const std::string& consumerName);

private:
    //! Helper method for reading value of type \c T from ::settings_reader
    template <typename T>
    T read(const char* path);

    std::unique_ptr<settings_reader> m_settingsReader;
};

template <typename... Args>
settings_view<Args...> settings_provider::get_view(const std::string& consumerName)
{
    return settings_view<Args...>(Args::parse(read<typename Args::source_type>(Args::path))...);
}

template <typename T>
T settings_provider::read(const char* path)
{
    T value{};
    m_settingsReader->get(value, path);

    return value;
}
