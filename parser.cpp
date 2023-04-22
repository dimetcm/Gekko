#include "parser.h"
#include "statements.h"
#include "expressions.h"
#include "gekko.h"
#include <assert.h>

Parser::Parser(const std::vector<Token>& tokens)
    : m_tokens(tokens)
{}

std::vector<IStatementPtr> Parser::Parse(std::ostream& logOutput)
{
    std::vector<IStatementPtr> programme;

    try
    {
        while (!Match(Token::Type::EndOfFile))
        {
           programme.push_back(ParseStatement());
        }
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

    return programme;
}

IExpressionPtr Parser::ParseBinaryExpression(std::function<IExpressionPtr()> exprFunc, std::initializer_list<Token::Type> tokenTypes)
{
    for (Token::Type tokenType : tokenTypes)
    {
        if (!CanBeUnary(tokenType) && Match(tokenType))
        {
            const Token& op = m_tokens[m_current++];
            exprFunc(); // discard right handed expression in case of an error
            throw ParseError(op, "Binary operator appearing at the beginning of an expression");
        }
    }

    IExpressionPtr left = exprFunc();
    while (MatchAny(tokenTypes))
    {
        const Token& op = m_tokens[m_current++];

        IExpressionPtr right = exprFunc();
        left = std::make_unique<BinaryExpression>(std::move(left), op, std::move(right));
    }
    
    return left;
}

IStatementPtr Parser::ParseStatement()
{
    return Match(Token::Type::Print) ? ParsePrintStatement() : ParseExpressionStatement();
}

IStatementPtr Parser::ParsePrintStatement()
{
    assert(Match(Token::Type::Print));
    ++m_current; // consume print

    IExpressionPtr value = ParseExpression();
    Consume(Token::Type::Semicolon, "Expect ';' after value.");

    return std::make_unique<PrintStatement>(std::move(value));
}

IStatementPtr Parser::ParseExpressionStatement()
{
    IExpressionPtr expression = ParseExpression();
    Consume(Token::Type::Semicolon, "Expect ';' after expression.");
    
    return std::make_unique<ExpressionStatement>(std::move(expression));
}

IExpressionPtr Parser::ParseExpression()
{
    return ParseComma();
} 

IExpressionPtr Parser::ParseComma()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseTernaryConditional, this), {Token::Type::Comma});
}

IExpressionPtr Parser::ParseTernaryConditional()
{
    IExpressionPtr expression = ParseEquality();

    while (Match(Token::Type::Questionmark))
    {
        ++m_current; // consume questionmark
        IExpressionPtr trueBranch = ParseExpression();

        if (Match(Token::Type::Colon))
        {
            ++m_current; // consume colon
            IExpressionPtr falseBranch = ParseExpression();
            expression = std::make_unique<TernaryConditionalExpression>(std::move(expression), std::move(trueBranch), std::move(falseBranch));
        }
        else
        {
            throw ParseError(m_tokens[m_current], "missing colon ':' after questionmark '?' in ternary conditional operator.");
        }
    }

    return expression;
}

IExpressionPtr Parser::ParseEquality()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseComparison, this), {Token::Type::BangEqual, Token::Type::EqualEqual});
} 

IExpressionPtr Parser::ParseComparison()
{    
    return ParseBinaryExpression(std::bind(&Parser::ParseTerm, this), {Token::Type::Greater, Token::Type::GreaterEqual, Token::Type::LessEqual, Token::Type::Less});
} 

IExpressionPtr Parser::ParseTerm()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseFactor, this), {Token::Type::Minus, Token::Type::Plus});
}

IExpressionPtr Parser::ParseFactor()
{
    return ParseBinaryExpression(std::bind(&Parser::ParseUnary, this), {Token::Type::Slash, Token::Type::Star});
}

IExpressionPtr Parser::ParseUnary()
{
    if (MatchAny({Token::Type::Bang, Token::Type::Minus}))
    {
        const Token& op = m_tokens[m_current++];
        return std::make_unique<UnaryExpression>(op, ParseUnary());
    }

    return ParsePrimary();
}

IExpressionPtr Parser::ParsePrimary()
{
    const Token& token = m_tokens[m_current++];
    switch (token.m_type)
    {
    case Token::Type::False:    return std::make_unique<LiteralExpression>(Value(false)); 
    case Token::Type::True:     return std::make_unique<LiteralExpression>(Value(true));
    case Token::Type::Nil:      return std::make_unique<LiteralExpression>();
    case Token::Type::Number:
    case Token::Type::String:   return std::make_unique<LiteralExpression>(Value(token.m_literalvalue));
    case Token::Type::OpeningParenthesis:
    {
        IExpressionPtr expression = ParseExpression();
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

void Parser::Consume(Token::Type tokenType, std::string&& errorMessage)
{
    if (Match(tokenType))
    {
        ++m_current;
    }
    else
    {
        throw ParseError(m_tokens[m_current], errorMessage);
    }
}

bool Parser::CanBeUnary(Token::Type tokenType) const
{
    return tokenType == Token::Type::Minus || tokenType == Token::Type::Plus;
}