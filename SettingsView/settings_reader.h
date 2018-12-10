#pragma once

#include <string>

class settings_reader
{
public:
    virtual ~settings_reader() = default;

    virtual void get(int& value, const std::string& path) = 0;
    virtual void get(int& value, const char* path) = 0;

    virtual void get(std::string& value, const std::string& path) = 0;
    virtual void get(std::string& value, const char* path) = 0;
};