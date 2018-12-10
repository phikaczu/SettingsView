#include "pch.h"

#include "json_settings_reader.h"
#include "settings_provider.h"
#include "settings_types.h"

#include <iostream>
#include <rapidjson/document.h>

int main()
{
    try
    {
        constexpr auto settingsJsonStr = " { \"name\" : \"Filip\", \"age\" : 110, \"salary\" : 0 } ";
        rapidjson::Document settingsJson;
        settingsJson.Parse(settingsJsonStr);

        auto settingsReader = std::make_unique<json_settings_reader>(std::move(settingsJson));
        settings_provider provider(std::move(settingsReader));

        auto token = provider.add_observer([](const std::string& consumer, const std::vector<std::string>& types) {
            std::cout << "consumer: '" << consumer << "' requested:";
            for (const auto& type : types)
            {
                std::cout << ' ' << type << ',';
            }
            std::cout << "\n\n";
        });

        const auto settingsView = provider.get_view<settings::age, settings::name, settings::salary>(__FUNCTION__);

        std::cout << "name    : " << settingsView.get<settings::name>() << std::endl;
        std::cout << "age     : " << settingsView.get<settings::age>() << std::endl;
        std::cout << "salary  : " << settingsView.get<settings::salary>() << std::endl;
    }
    // Exception will be thrown e.g. when the settings_reader does not find the requested path
    catch (const std::exception& ex)
    {
        std::cout << "Exception cought: " << ex.what() << std::endl;
    }
}