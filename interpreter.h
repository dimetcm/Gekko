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
        Environment(Environment* outer = nullptr);

        void Define(const Token& token, const Value& value);
        void Assign(const Token& token, const Value& value);
        Value GetValue(const Token& token) const;

        Environment(const Environment&) = delete;
        Environment& operator=(const Environment&) = delete;
    protected:
        std::map<std::string, Value> m_values;
        Environment* m_outer = nullptr;
    };

    void Interpret(Environment& environment, const std::vector<IStatementPtr>& program, std::ostream& outputStream, std::ostream& errorsLog) const; 
protected:
    struct StatementVisitorContext : IStatementVisitorContext
    {
        StatementVisitorContext(Environment& environment, std::ostream& outputStream)
            : m_environment(environment)
            , m_outputStream(outputStream) {}
        Environment& m_environment;
        std::ostream& m_outputStream;
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
    virtual void VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitIfStatement(const IfStatement& statement, IStatementVisitorContext* context) const override;

    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const override;

    void Execute(const IStatement& statement, Environment& environment, std::ostream& outputStream) const;
    Value Eval(const IExpression& expression, Environment& environment) const;

    static bool AreEqual(const Token& token, const Value& lhs, const Value& rhs);
    static double GetNumberOperand(const Token& token, const Value& lhs);

    static Environment& GetEnvironment(IExpressionVisitorContext& context);
    static Environment& GetEnvironment(IStatementVisitorContext& context);
    static std::ostream& GetOutputStream(IStatementVisitorContext& context);
};
