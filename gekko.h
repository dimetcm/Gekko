#pragma once

#include <string>

struct Token;

class Gekko
{
public:
    static void ReportError(const Token& token, std::string_view message);
    static void ReportError(int line, std::string_view message);
    static void ReportError(int line, std::string_view where, std::string_view message);
};