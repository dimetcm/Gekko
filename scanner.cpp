#include "scanner.h"
#include <charconv>
#include "gekko.h"

Scanner::Scanner(std::string_view source)
    : m_source(source)
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
        case ':': AddToken(Token::Type::Colon); break;
        case ';': AddToken(Token::Type::Semicolon); break;
        case '*': AddToken(Token::Type::Star); break;
        case '?': AddToken(Token::Type::Questionmark); break;

        case '!': AddToken(AdvanceIfMatch('=') ? Token::Type::BangEqual : Token::Type::Bang); break;
        case '=': AddToken(AdvanceIfMatch('=') ? Token::Type::EqualEqual : Token::Type::Equal); break;
        case '<': AddToken(AdvanceIfMatch('=') ? Token::Type::LessEqual : Token::Type::Less); break;
        case '>': AddToken(AdvanceIfMatch('=') ? Token::Type::GreaterEqual : Token::Type::Greater); break;

        case '/': 
        {
            if (AdvanceIfMatch('/')) // // comments
            {
                while (Peek() != '\n' && !IsAtEnd())
                {
                    Advance();
                }
            }
            else if (AdvanceIfMatch('*')) // /* comments
            {
                while (!IsAtEnd())
                {
                    if (AdvanceIfMatch('*') && Peek() == '/')
                    {
                        Advance(); // consume '/'
                        break;
                    }
                    else if (Peek() == '\n')
                    {
                        ++m_line;
                    }

                    Advance();
                }
                
                if (IsAtEnd())
                {
                    Gekko::ReportError(m_line, "Unterminated comment block.");
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
            else if (IsAlpha(c))
            {
                ScanIdentifier();
            }
            else
            {
                Gekko::ReportError(m_line, "Unexpected character.");
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
        Gekko::ReportError(m_line, "Unterminated string.");
        return;
    }
    
    Advance(); // closing "

    AddToken(Token::Type::String, std::string(m_source.substr(m_start + 1, m_current - m_start - 2)));
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

bool Scanner::AdvanceIfMatch(char match)
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

bool Scanner::IsAlpha(char c) const
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::IsAlphanNmeric(char c) const
{
    return IsAlpha(c) || IsDigit(c);
}

void Scanner::ScanNumberLiteral()
{
    while (IsDigit(Peek()))
    {
        Advance();
    }

    if (Peek() == '.')
    {
        Advance(); // consume '.'
        
        while (IsDigit(Peek()))
        {
            Advance();
        }        
    }

    std::string_view numberStr = m_source.substr(m_start, m_current-m_start);
    double value = 0.0;
    if (std::from_chars(numberStr.data(), numberStr.data() + numberStr.size(), value).ec == std::errc{})
    {
        AddToken(Token::Type::Number, value);  
    }
    else
    {
        Gekko::ReportError(m_line, "Can't scan number literal.");   
    }
}

void Scanner::ScanIdentifier()
{
    while (IsAlphanNmeric(Peek()))
    {
        Advance();
    }
    
    Token::Type type = Token::Type::Identifier;
    std::string_view keyword = m_source.substr(m_start, m_current-m_start);
    StaticData::Instance().KeywordToTokenType(keyword, type);

    AddToken(type);
}

Scanner::StaticData::StaticData()
{
    m_keywordToTokenType.insert({"return", Token::Type::Return});
    m_keywordToTokenType.insert({"nil", Token::Type::Nil});
    m_keywordToTokenType.insert({"false", Token::Type::False});
    m_keywordToTokenType.insert({"true", Token::Type::True});
    m_keywordToTokenType.insert({"and", Token::Type::And});
    m_keywordToTokenType.insert({"or", Token::Type::Or});
    m_keywordToTokenType.insert({"if", Token::Type::If});
    m_keywordToTokenType.insert({"else", Token::Type::Else});
    m_keywordToTokenType.insert({"for", Token::Type::For});
    m_keywordToTokenType.insert({"while", Token::Type::While});
    m_keywordToTokenType.insert({"fun", Token::Type::Fun});
    m_keywordToTokenType.insert({"var", Token::Type::Var});
    m_keywordToTokenType.insert({"class", Token::Type::Class});
    m_keywordToTokenType.insert({"this", Token::Type::This});
    m_keywordToTokenType.insert({"super", Token::Type::Super});
    m_keywordToTokenType.insert({"print", Token::Type::Print});
}

bool Scanner::StaticData::KeywordToTokenType(std::string_view keyword, Token::Type& outType) const
{
    const auto& it = m_keywordToTokenType.find(keyword);
    if (it == m_keywordToTokenType.cend())
    {
        return false; 
    }

    outType = it->second;
    return true;
}
