#include "pch.h"

#include <iostream>
#include "settings_provider.h"
#include "settings_types.h"

int main()
{
    try
    {
        constexpr auto settingsJsonStr = " { \"name\" : \"Filip\", \"age\" : 110 } ";
        rapidjson::Document settingsJson;
        settingsJson.Parse(settingsJsonStr);

        settings_provider provider(std::move(settingsJson));
        const auto settingsView = provider.get_view<settings::age, settings::name /*, settings::salary*/>(__FUNCTION__);

        std::cout << "name    : " << settingsView.get<settings::name>() << std::endl;
        std::cout << "age     : " << settingsView.get<settings::age>() << std::endl;
        // std::cout << "salary  : " << settingsView.get<settings::salary>() << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception cought: " << ex.what() << std::endl;
    }
}

//// It would be possible to impement an settings_reader abstraction as follows
// class settings_reader
//{
//    template<typename T>
//    T get(const char* path);
//};
//
//// It would be used in the settings_provider::get() method as
// template <typename T, typename... Args>
// inline settings_view<Args...> settings_provider::get_view(T& reader, const std::string& consumerName)
//{
//    return settings_view<Args...>(reader.get<Args::value_type>()...);
//}

//// Or
// class settings_reader
//{
//    virtual Value get(const char* path) = 0;
//};
//// Where Value provides type erasure

//// Or the whole class could be templated eith the settings_reader type

//// Nebo se muze definovat mnozina hodnot, ktere se daji ulozit do konfiguraku (int, string, double?)
//// a parse metody v settings_types.h budou prijmat jen tyto hodnoty, takze se oddeli misto, kde se bude
//// dostavat hodnota ze souboru a misto, kde se bude prevadet na ocekavany typ
//// settings_reader interface bude pak definovat jen void get(typ&) pro vsechny vyjmenovane typy
//// a settings_provider::get_view bude muset mit pomocnou statickou metodu ktera mu umozny vybrat
//// spravnou metodu readeru podle typu
//template <typename T>
//T get()
//{
//    T value{};
//    m_reader.get(value);
//    return value;
//}