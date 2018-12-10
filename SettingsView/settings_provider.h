#pragma once

#include "callback_container.h"
#include "settings_reader.h"
#include "settings_view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

class settings_provider
{
public:
    using observer_callback_t = std::function<void(const std::string&, const std::vector<std::string>&)>;

private:
    using callback_container_t = callback_container<observer_callback_t>;
    using observer_container_t = std::shared_ptr<callback_container_t>;
    using observer_token_t = typename callback_container_t::token_t;

public:
    explicit settings_provider(std::unique_ptr<settings_reader>&& settingsReader);

    template <typename... Args>
    settings_view<Args...> get_view(const std::string& consumerName);

    observer_token_t add_observer(observer_callback_t&& callback);

private:
    //! Helper method for reading value of type \c T from ::settings_reader
    template <typename T>
    T read(const char* path);

    std::unique_ptr<settings_reader> m_settingsReader;
    observer_container_t m_observers;
};

template <typename... Args>
settings_view<Args...> settings_provider::get_view(const std::string& consumerName)
{
    // TODO typeid(Args).name() does not need to be human readable
    const std::vector<std::string> types{typeid(Args).name()...};
    (*m_observers)(consumerName, types);

    return settings_view<Args...>(Args::parse(read<typename Args::source_type>(Args::path))...);
}

template <typename T>
T settings_provider::read(const char* path)
{
    T value{};
    m_settingsReader->get(value, path);

    return value;
}
