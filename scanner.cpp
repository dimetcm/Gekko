#include "scanner.h"

Scanner::Scanner(std::string_view source, IErrorReporter& errorReporter)
    : m_source(source)
    , m_errorReporter(errorReporter)
{
    ScanTokens();
}

void Scanner::ScanTokens()
{
    while (!IsAtEnd())
    {
        m_start = m_current;
        ScanToken();    
    }

    m_tokens.emplace_back(Token::Type::EndOfFile, "", std::any(), m_line);
}

void Scanner::ScanToken()
{
    char c = Advance();
    switch (c)
    {
        case '(': AddToken(Token::Type::OpeningParenthesis); break;
        case ')': AddToken(Token::Type::ClosingParenthesis); break;
        case '{': AddToken(Token::Type::OpeningBrace); break;
        case '}': AddToken(Token::Type::ClosingBrace); break;    
        case ',': AddToken(Token::Type::Comma); break;
        case '.': AddToken(Token::Type::Dot); break;
        case '-': AddToken(Token::Type::Minus); break;
        case '+': AddToken(Token::Type::Plus); break;
        case ';': AddToken(Token::Type::Semicolon); break;
        case '*': AddToken(Token::Type::Star); break;

        case '!': AddToken(AdvanceIfMatch('=') ? Token::Type::BangEqual : Token::Type::Bang); break;
        case '=': AddToken(AdvanceIfMatch('=') ? Token::Type::EqualEqual : Token::Type::Equal); break;
        case '<': AddToken(AdvanceIfMatch('=') ? Token::Type::LessEqual : Token::Type::Less); break;
        case '>': AddToken(AdvanceIfMatch('=') ? Token::Type::GreaterEqual : Token::Type::Greater); break;

        case '/': 
        {
            if (AdvanceIfMatch('/'))
            {
                while (Peek() != '\n' && !IsAtEnd())
                {
                    Advance();
                }
            }
            else
            {
                AddToken(Token::Type::Slash);
            }
        }

        case ' ':
        case '\r':
        case '\t': /*ignore whitespaces*/ break;

        case '\n': ++m_line; break;

        case '"': ScanStringLiteral(); break;

        default:
        {
            if (IsDigit(c))
            {
                ScanNumberLiteral();
            }
            else
            {
                m_errorReporter.OnError(m_line, "Unexpected character.");
            }
        }   
    }
}

void Scanner::AddToken(Token::Type tokenType)
{
    AddToken(tokenType, std::any());
}

void Scanner::AddToken(Token::Type tokenType, std::any literalValue)
{
    std::string_view lexeme = m_source.substr(m_start, m_current-m_start);
    m_tokens.emplace_back(tokenType, lexeme, literalValue, m_line);
}

void Scanner::ScanStringLiteral()
{
    while (!IsAtEnd() && Peek() != '"')
    {
        if (Peek() == '\n')
        {
            ++m_line;
        }
        Advance();
    }

    if(IsAtEnd())
    {
        m_errorReporter.OnError(m_line, "Unterminated string.");
        return;
    }
    
    Advance(); // closing "

    AddToken(Token::Type::String, m_source.substr(m_start + 1, m_current - m_start - 1));
}

bool Scanner::IsAtEnd() const
{
    return m_current >= m_source.length(); 
}

char Scanner::Peek() const
{
    return IsAtEnd() ? '\0' : m_source[m_current];
}

char Scanner::Advance()
{
    return m_source[m_current++];
}

char Scanner::AdvanceIfMatch(char match)
{
    if (IsAtEnd() || m_source[m_current] != match)
    {
        return false;
    }

    ++m_current;
    return true;
}

bool Scanner::IsDigit(char c) const
{
    return c >= '0' && c <= '9';
}

void Scanner::ScanNumberLiteral()
{
    int integralPart = 0;

    while (IsDigit(Peek()))
    {
        integralPart = integralPart * 10 + Peek() - '0';
        Advance();
    }

    double fractionalPart = 0;
    double p = 0.1;

    if (Peek() == '.')
    {
        Advance(); // consume '.'
        
        while (IsDigit(Peek()))
        {
            fractionalPart += p * (Peek() - '0');
            p *= 0.1;
            Advance();
        }        
    }

    AddToken(Token::Type::Number, integralPart + fractionalPart);  
}
