#include "pch.h"

#include "settings_provider.h"

settings_provider::settings_provider(std::unique_ptr<settings_reader>&& settingsReader)
    : m_settingsReader{ std::move(settingsReader) }
{
}
