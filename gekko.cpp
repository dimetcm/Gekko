#include "gekko.h"
#include "token.h"
#include <iostream>

void Gekko::ReportError(const Token& token, std::string_view message)
{
    if (token.m_type == Token::Type::EndOfFile) 
    {
        ReportError(token.m_line, " at end", message);
    }
    else 
    {
        std::string lexeme = " at '" + std::string(token.m_lexeme) + "'";
        ReportError(token.m_line, lexeme, message);
    }
}

void Gekko::ReportError(int line, std::string_view message)
{
    ReportError(line, "", message);
}

void Gekko::ReportError(int line, std::string_view where, std::string_view message)
{
    std::cerr << "[line " << line << "] Error " << where << ": " << message << std::endl; 
}
