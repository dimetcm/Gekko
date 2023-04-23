#pragma once 

#include <vector>
#include <functional>
#include "token.h"

class IExpression;
using IExpressionPtr = std::unique_ptr<const IExpression>;

struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

// consumes an array of tokens and produces a programm (for now an array of statements).
// uses recursive descent for it. 
class Parser
{
public:
    Parser(const std::vector<Token>& tokens);

    std::vector<IStatementPtr> Parse(std::ostream& logOutput);
protected:
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
    bool Match(Token::Type tokenType) const;
    const Token& Consume(Token::Type tokenType, std::string&& errorMessage);

    bool CanBeUnary(Token::Type tokenType) const;
    const Token& CurrentToken() const;
 
    IStatementPtr ParseDeclaration();
    IStatementPtr ParseVariableDeclaration();
    IStatementPtr ParseStatement();
    IStatementPtr ParsePrintStatement();
    IStatementPtr ParseExpressionStatement();
    IExpressionPtr ParseExpression();
    IExpressionPtr ParseComma();
    IExpressionPtr ParseTernaryConditional();
    IExpressionPtr ParseEquality();
    IExpressionPtr ParseComparison();
    IExpressionPtr ParseTerm();
    IExpressionPtr ParseFactor();
    IExpressionPtr ParseUnary();
    IExpressionPtr ParsePrimary();

    void Synchronize();


    int m_current = 0;
    const std::vector<Token>& m_tokens; 
};