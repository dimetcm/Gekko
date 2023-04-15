#pragma once 

#include <vector>
#include <functional>
#include "token.h"

class IExpression;
using IExpressionPtr = std::unique_ptr<const IExpression>;

// consumes an array of tokens and builds an expression out of them.
// uses recursive descent for it. 
class Parser
{
public:
    Parser(const std::vector<Token>& tokens);

    IExpressionPtr Parse(std::ostream& logOutput);
private:
    struct ParseError : std::exception
    {
        ParseError(const Token& token, std::string message)
            : m_token(token)
            , m_message(std::move(message))
        {}

        const char* what () const throw () { return "Parse error."; }

        const Token& m_token;
        std::string m_message;
    };

    IExpressionPtr ParseBinaryExpression(std::function<IExpressionPtr()> exprFunc, std::initializer_list<Token::Type> tokenTypes);
    bool MatchAny(std::initializer_list<Token::Type> tokenTypes) const;
    bool Match(Token::Type typetokenType) const;

 
    IExpressionPtr Expression();
    IExpressionPtr Equality();
    IExpressionPtr Comparison();
    IExpressionPtr Term();
    IExpressionPtr Factor();
    IExpressionPtr Unary();
    IExpressionPtr Primary();

private:
    int m_current = 0;
    const std::vector<Token>& m_tokens; 
};