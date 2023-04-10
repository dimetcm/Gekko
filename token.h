#pragma once

#include <ostream>
#include <any>

struct Token
{
    enum class Type
    {
        OpeningParenthesis,         // '('
        ClosingParenthesis,         // ')'
        OpeningBrace,               // '{'
        ClosingBrace,               // '}'
        Comma,                      // ','
        Dot,                        // '.'
        Minus,                      // '-'
        Plus,                       // '+'
        Semicolon,                  // ';'
        Slash,                      // '/'
        Star,                       // '*'

        Bang,                       // '!'
        BangEqual,                  // '!='
        Equal,                      // '='
        EqualEqual,                 // '=='
        Greater,                    // '>'
        GreaterEqual,               // '>='
        Less,                       // '<'
        LessEqual,                  // '<='

        // literals
        Identifier,
        String,
        Number,

        // Keywords
        Return,
        Nil,
        False,
        True,
        And,
        Or,
        If,
        Else,
        For,
        While,
        Fun,
        Var,
        Class,
        This,
        Super,
        Print,

        EndOfFile
    };

    Token(Type type, std::string_view lexeme, std::any literalValue, int line)
        : m_type(type)
        , m_lexeme(lexeme)
        , m_literalvalue(literalValue)
        , m_line(line)
    {}

    Type m_type;
    std::string_view m_lexeme;
    std::any m_literalvalue;
    int m_line;
};

std::string_view TokenTypeToStringView(Token::Type tokenType);

std::ostream& operator<<(std::ostream& os, const Token& token);
