#pragma once

#include <string>
#include <vector>

#include "token.h"


class Scanner
{
public:
    class IErrorReporter
    {
    protected:
        virtual void OnError(int line, std::string_view message) = 0;
        virtual ~IErrorReporter() {}

        friend Scanner;
    };
public:
    Scanner(std::string_view source, IErrorReporter& errorReporter);

    const std::vector<Token>& Tokens() const { return m_tokens; }

private:
    void ScanTokens();
    void ScanToken();
    void AddToken(Token::Type tokenType);
    void AddToken(Token::Type tokenType, std::any literalValue);

    void ScanStringLiteral();
    void ScanNumberLiteral();
    void ScanIdentifier();

    char Advance();
    char AdvanceIfMatch(char match);
    char Peek() const;
    bool IsAtEnd() const;

    bool IsDigit(char c) const;
    bool IsAlpha(char c) const;
    bool IsAlphanNmeric(char c) const;

    std::string_view m_source;
    IErrorReporter& m_errorReporter;
    std::vector<Token> m_tokens;

    int m_start = 0;
    int m_current = 0;
    int m_line = 1;
};