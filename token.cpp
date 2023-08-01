#include "token.h"

std::string_view TokenTypeToStringView(Token::Type tokenType)
{
    switch (tokenType)
    {
        case Token::Type::OpeningParenthesis:   { static std::string str("("); return str; }
        case Token::Type::ClosingParenthesis:   { static std::string str(")"); return str; }
        case Token::Type::OpeningBrace:         { static std::string str("{"); return str; }
        case Token::Type::ClosingBrace:         { static std::string str("}"); return str; }
        case Token::Type::Comma:                { static std::string str(","); return str; }
        case Token::Type::Dot:                  { static std::string str("."); return str; }
        case Token::Type::Minus:                { static std::string str("-"); return str; }
        case Token::Type::Plus:                 { static std::string str("+"); return str; }
        case Token::Type::Colon:                { static std::string str(":"); return str; }
        case Token::Type::Semicolon:            { static std::string str(";"); return str; }
        case Token::Type::Slash:                { static std::string str("/"); return str; }
        case Token::Type::Star:                 { static std::string str("*"); return str; }
        case Token::Type::Questionmark:         { static std::string str("?"); return str; }

        case Token::Type::Bang:                 { static std::string str("!"); return str; }
        case Token::Type::BangEqual:            { static std::string str("!="); return str; }
        case Token::Type::Equal:                { static std::string str("="); return str; }
        case Token::Type::EqualEqual:           { static std::string str("=="); return str; }
        case Token::Type::Greater:              { static std::string str(">"); return str; }
        case Token::Type::GreaterEqual:         { static std::string str(">="); return str; }
        case Token::Type::Less:                 { static std::string str("<"); return str; }
        case Token::Type::LessEqual:            { static std::string str("<="); return str; }

        // literals
        case Token::Type::Identifier:           { static std::string str("identifier"); return str; }
        case Token::Type::String:               { static std::string str("string"); return str; }
        case Token::Type::Number:               { static std::string str("number"); return str; }

        // Keywords
        case Token::Type::Return:               { static std::string str("return"); return str; }
        case Token::Type::Nil:                  { static std::string str("nil"); return str; }
        case Token::Type::False:                { static std::string str("false"); return str; }
        case Token::Type::True:                 { static std::string str("true"); return str; }
        case Token::Type::And:                  { static std::string str("and"); return str; }
        case Token::Type::Or:                   { static std::string str("or"); return str; }
        case Token::Type::If:                   { static std::string str("if"); return str; }
        case Token::Type::Else:                 { static std::string str("else"); return str; }
        case Token::Type::For:                  { static std::string str("for"); return str; }
        case Token::Type::While:                { static std::string str("while"); return str; }
        case Token::Type::Break:                { static std::string str("break"); return str; }
        case Token::Type::Fun:                  { static std::string str("fun"); return str; }
        case Token::Type::Var:                  { static std::string str("var"); return str; }
        case Token::Type::Class:                { static std::string str("class"); return str; }
        case Token::Type::This:                 { static std::string str("this"); return str; }
        case Token::Type::Super:                { static std::string str("super"); return str; }
        case Token::Type::Print:                { static std::string str("print"); return str; }

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
        os << " " << *token.m_literalvalue.GetNumber();
    }
    else if (token.m_type == Token::Type::String)
    {
        os << " " << *token.m_literalvalue.GetString();
    }

    return os;
}
