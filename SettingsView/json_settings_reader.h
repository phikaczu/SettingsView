#pragma once

#include "settings_reader.h"

#include <rapidjson/document.h>
#include <memory>

class json_settings_reader final : public settings_reader
{
public:
    explicit json_settings_reader(rapidjson::Document&& settings);

    void get(int& value, const std::string& path) override;
    void get(int& value, const char* path) override;

    void get(std::string& value, const std::string& path) override;
    void get(std::string& value, const char* path) override;

private:
    rapidjson::Document m_settings;
};