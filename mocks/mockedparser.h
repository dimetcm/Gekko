#pragma once

#include "../parser.h"
#include "../gekko.h"

struct MockedParser : Parser
{
    MockedParser(const std::vector<Token>& tokens)
        : Parser(tokens)
    {}

    IExpressionPtr ParseExpression(std::ostream& logOutput)
    {
        try
        {
            return Parser::ParseExpression();
        }
        catch(const ParseError& pe)
        {
            if (pe.m_token.m_type == Token::Type::EndOfFile)
            {
                Gekko::ReportError(pe.m_token.m_line, " at end", pe.m_message);
            }
            else
            {
                Gekko::ReportError(pe.m_token.m_line, " at '" + std::string(pe.m_token.m_lexeme) + "'", pe.m_message);
            }
        }

        return nullptr;
    }
};