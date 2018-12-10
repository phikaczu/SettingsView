#include "pch.h"

#include "settings_provider.h"

settings_provider::settings_provider(std::unique_ptr<settings_reader>&& settingsReader)
    : m_settingsReader{ std::move(settingsReader) }
    , m_observers{ callback_container_t::create_callback_container() }
{
}

settings_provider::observer_token_t settings_provider::add_observer(observer_callback_t&& callback)
{
    return m_observers->register_callback(std::move(callback));
}


