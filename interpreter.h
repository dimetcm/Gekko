#pragma once

#include <any>
#include <iostream>
#include <vector>
#include <map>
#include "expressionvisitor.h"
#include "statementvisitor.h"
#include "value.h"

struct Token;
class IExpression;
struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

struct Interpreter : IExpressionVisitor, IStatementVisitor
{
    struct Environment
    {
        void Define(const std::string& name, const Value& value);
        const Value* GetValue(const std::string& name) const;
    private:
        std::map<std::string, Value> m_values;
    };

    void Interpret(Environment& environment, const std::vector<IStatementPtr>& program, std::ostream& logOutput) const; 
protected:
    struct StatementVisitorContext : IStatementVisitorContext
    {
        StatementVisitorContext(Environment& environment) : m_environment(environment) {}
        Environment& m_environment;
    };

    struct ExpressionVisitorContext : IExpressionVisitorContext 
    {
        ExpressionVisitorContext(Environment& environment) : m_environment(environment) {} 
        Environment& m_environment;
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
    virtual void VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const override;

    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const override;

    void Execute(const IStatement& statement, Environment& environment) const;
    Value Eval(const IExpression& expression, Environment& environment) const;

    static bool AreEqual(const Token& token, const Value& lhs, const Value& rhs);
    static double GetNumberOperand(const Token& token, const Value& lhs);

    static Environment& GetEnvironment(IExpressionVisitorContext& context);
    static Environment& GetEnvironment(IStatementVisitorContext& context);
};
