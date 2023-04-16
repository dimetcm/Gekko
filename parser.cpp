#include "parser.h"
#include "expressions.h"
#include "gekko.h"

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens)
{}

IExpressionPtr Parser::Parse(std::ostream& logOutput)
{
    try
    {
        return Expression();
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
    catch(const std::exception& e)
    {
        logOutput << e.what() << '\n';
    }

    return nullptr;
}

IExpressionPtr Parser::ParseBinaryExpression(std::function<IExpressionPtr()> exprFunc, std::initializer_list<Token::Type> tokenTypes)
{
   IExpressionPtr left = exprFunc();
    while (MatchAny(tokenTypes))
    {
        const Token& op = m_tokens[m_current++];

        IExpressionPtr right = exprFunc();
        left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
    }
    
    return left;
}

IExpressionPtr Parser::Expression()
{
    return Comma();
} 

IExpressionPtr Parser::Comma()
{
    return TernaryConditional();
}

IExpressionPtr Parser::TernaryConditional()
{
    IExpressionPtr expression = Equality();

    while (Match(Token::Type::Questionmark))
    {
        ++m_current; // consume questionmark
        IExpressionPtr trueBranch = Expression();

        if (Match(Token::Type::Colon))
        {
            ++m_current; // consume colon
            IExpressionPtr falseBranch = Expression();
            expression = std::make_unique<TernaryConditionalExpression>(std::move(expression), std::move(trueBranch), std::move(falseBranch));
        }
        else
        {
            throw ParseError(m_tokens[m_current], "missing colon ':' after questionmark '?' in ternary conditional operator.");
        }
    }

    return expression;
}

IExpressionPtr Parser::Equality()
{
    return ParseBinaryExpression(std::bind(&Parser::Comparison, this), {Token::Type::BangEqual, Token::Type::EqualEqual});
} 

IExpressionPtr Parser::Comparison()
{    
    return ParseBinaryExpression(std::bind(&Parser::Term, this), {Token::Type::Greater, Token::Type::GreaterEqual, Token::Type::LessEqual, Token::Type::Less});

} 

IExpressionPtr Parser::Term()
{
    return ParseBinaryExpression(std::bind(&Parser::Factor, this), {Token::Type::Minus, Token::Type::Plus});
}

IExpressionPtr Parser::Factor()
{
    return ParseBinaryExpression(std::bind(&Parser::Unary, this), {Token::Type::Slash, Token::Type::Star});
}

IExpressionPtr Parser::Unary()
{
    if (MatchAny({Token::Type::Bang, Token::Type::Minus}))
    {
        const Token& op = m_tokens[m_current++];
        return std::make_unique<UnaryExpression>(op, Unary());
    }

    return Primary();
}

IExpressionPtr Parser::Primary()
{
    const Token& token = m_tokens[m_current++];
    switch (token.m_type)
    {
    case Token::Type::False:    return std::make_unique<LiteralExpression>(std::make_any<bool>(false)); 
    case Token::Type::True:     return std::make_unique<LiteralExpression>(std::make_any<bool>(true));
    case Token::Type::Nil:      return std::make_unique<LiteralExpression>(std::make_any<nullptr_t>(nullptr));
    case Token::Type::Number:
    case Token::Type::String:   return std::make_unique<LiteralExpression>(token.m_literalvalue);
    case Token::Type::OpeningParenthesis:
    {
        IExpressionPtr expression = Expression();
        if (Match(Token::Type::ClosingParenthesis))
        {
            ++m_current; // consume ClosingParenthesis
            return std::make_unique<GroupingExpression>(std::move(expression));
        }
        else
        {
            throw ParseError(token, "Expect ')' after expression.");
        }
    }
    default: throw ParseError(token, "Unhandled primary token type.");
    }
}

bool Parser::MatchAny(std::initializer_list<Token::Type> tokenTypes) const
{
    for (Token::Type tokenType : tokenTypes)
    {
        if (m_tokens[m_current].m_type == tokenType)
        {
            return true;
        }            
    } 

    return false;   
}

bool Parser::Match(Token::Type tokenType) const
{
    return m_tokens[m_current].m_type == tokenType;
}