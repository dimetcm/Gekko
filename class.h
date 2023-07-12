#pragma once

#include <string_view>

class Class
{
public:
    explicit Class(std::string_view name) : m_name(name) {}

    std::string_view ToString() const { return m_name; }

private:
    std::string_view m_name;
};