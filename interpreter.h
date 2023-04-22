#pragma once

#include <any>
#include <iostream>
#include <vector>
#include "expressionvisitor.h"
#include "statementvisitor.h"
#include "value.h"

struct Token;
class IExpression;
struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

struct Interpreter : IExpressionVisitor, IStatementVisitor
{
    void Interpret(const std::vector<IStatementPtr>& program, std::ostream& logOutput) const; 
protected:
    struct Context : IExpressionVisitorContext, IStatementVisitorContext 
    {
        Value m_result;
    };

    struct InterpreterError : std::exception
    {
        InterpreterError(const Token& op, std::string message)
            : m_operator(op)
            , m_message(std::move(message))
        {}

        std::string m_message;
        const Token& m_operator;
    };
    
    virtual void VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const override;

    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override;

    void Execute(const IStatement& statement) const;
    Value Eval(const IExpression& expression) const;

    static bool AreEqual(const Token& token, const Value& lhs, const Value& rhs);
    static double GetNumberOperand(const Token& token, const Value& lhs);
};
