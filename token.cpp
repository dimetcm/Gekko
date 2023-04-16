#include "token.h"

std::string_view TokenTypeToStringView(Token::Type tokenType)
{
    switch (tokenType)
    {
        case Token::Type::OpeningParenthesis:   { static std::string str("OpeningParenthesis"); return str; }
        case Token::Type::ClosingParenthesis:   { static std::string str("ClosingParenthesis"); return str; }
        case Token::Type::OpeningBrace:         { static std::string str("OpeningBrace"); return str; }
        case Token::Type::ClosingBrace:         { static std::string str("ClosingBrace"); return str; }
        case Token::Type::Comma:                { static std::string str("Comma"); return str; }
        case Token::Type::Dot:                  { static std::string str("Dot"); return str; }
        case Token::Type::Minus:                { static std::string str("Minus"); return str; }
        case Token::Type::Plus:                 { static std::string str("Plus"); return str; }
        case Token::Type::Colon:                { static std::string str("Colon"); return str; }
        case Token::Type::Semicolon:            { static std::string str("Semicolon"); return str; }
        case Token::Type::Slash:                { static std::string str("Slash"); return str; }
        case Token::Type::Star:                 { static std::string str("Star"); return str; }
        case Token::Type::Questionmark:         { static std::string str("Questionmark"); return str; }

        case Token::Type::Bang:                 { static std::string str("Bang"); return str; }
        case Token::Type::BangEqual:            { static std::string str("BangEqual"); return str; }
        case Token::Type::Equal:                { static std::string str("Equal"); return str; }
        case Token::Type::EqualEqual:           { static std::string str("EqualEqual"); return str; }
        case Token::Type::Greater:              { static std::string str("Greater"); return str; }
        case Token::Type::GreaterEqual:         { static std::string str("GreaterEqual"); return str; }
        case Token::Type::Less:                 { static std::string str("Less"); return str; }
        case Token::Type::LessEqual:            { static std::string str("LessEqual"); return str; }

        // literals
        case Token::Type::Identifier:           { static std::string str("Identifier"); return str; }
        case Token::Type::String:               { static std::string str("String"); return str; }
        case Token::Type::Number:               { static std::string str("Number"); return str; }

        // Keywords
        case Token::Type::Return:               { static std::string str("Return"); return str; }
        case Token::Type::Nil:                  { static std::string str("Nil"); return str; }
        case Token::Type::False:                { static std::string str("False"); return str; }
        case Token::Type::True:                 { static std::string str("True"); return str; }
        case Token::Type::And:                  { static std::string str("And"); return str; }
        case Token::Type::Or:                   { static std::string str("Or"); return str; }
        case Token::Type::If:                   { static std::string str("If"); return str; }
        case Token::Type::Else:                 { static std::string str("Else"); return str; }
        case Token::Type::For:                  { static std::string str("For"); return str; }
        case Token::Type::While:                { static std::string str("While"); return str; }
        case Token::Type::Fun:                  { static std::string str("Fun"); return str; }
        case Token::Type::Var:                  { static std::string str("Var"); return str; }
        case Token::Type::Class:                { static std::string str("Class"); return str; }
        case Token::Type::This:                 { static std::string str("This"); return str; }
        case Token::Type::Super:                { static std::string str("Super"); return str; }
        case Token::Type::Print:                { static std::string str("Print"); return str; }

        case Token::Type::EndOfFile:            { static std::string str("EndOfFile"); return str; }
    }

    static std::string_view str("Unsuported Token type");
    return str;
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    os << TokenTypeToStringView(token.m_type) << " " << token.m_lexeme;
    if (token.m_type == Token::Type::Number)
    {
        os << " " << std::any_cast<double>(token.m_literalvalue);
    }
    else if (token.m_type == Token::Type::String)
    {
        os << " " << std::any_cast<std::string_view>(token.m_literalvalue);
    }

    return os;
}
