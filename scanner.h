#pragma once

#include <string>
#include <vector>
#include <map>

#include "token.h"


class Scanner
{
public:
    Scanner(std::string_view source);

    const std::vector<Token>& Tokens() const { return m_tokens; }

private:
    void ScanTokens();
    void ScanToken();
    void AddToken(Token::Type tokenType);
    void AddToken(Token::Type tokenType, Value literalValue);

    void ScanStringLiteral();
    void ScanNumberLiteral();
    void ScanIdentifier();

    char Advance();
    char Peek() const;
    bool AdvanceIfMatch(char match);
    bool IsAtEnd() const;

    bool IsDigit(char c) const;
    bool IsAlpha(char c) const;
    bool IsAlphanNmeric(char c) const;

    class StaticData 
    {
    public:
        static const StaticData& Instance() { static const StaticData data; return data; }

        bool KeywordToTokenType(std::string_view keyword, Token::Type& outType) const;

    private:
        StaticData();
    private:
        std::map<std::string_view, Token::Type> m_keywordToTokenType;
    };
    

    std::string_view m_source;
    std::vector<Token> m_tokens;

    int m_start = 0;
    int m_current = 0;
    int m_line = 1;
};