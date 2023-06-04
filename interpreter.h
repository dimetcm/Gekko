#pragma once

#include "expressionvisitor.h"
#include "statementvisitor.h"
#include "value.h"
#include <any>
#include <iostream>
#include <vector>
#include <map>
#include <memory>

struct Token;
struct IExpression;
struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

struct Environment;
using EnvironmentPtr = std::shared_ptr<Environment>;

struct Environment : std::enable_shared_from_this<Environment>
{
    static EnvironmentPtr CreateGlobalEnvironment(std::ostream& outputStream = std::cout);
    static EnvironmentPtr CreateLocalEnvironment(EnvironmentPtr outer);

    ~Environment();

    void Define(std::string_view name, const Value& value);
    void Assign(const Token& token, const Value& value);
    Value GetValue(const Token& token) const;
    Value GetValue(const Token& token, size_t distance) const;
    
    void RequestBreak();
    void ClearBreak();
    bool BreakRequested() const;

    void RequestReturn(Value returnValue);
    void ClearReturn();
    bool ReturnRequested() const;
    const Value& GetReturnValue() const;

    EnvironmentPtr GetGlobalEnvironment();

    std::ostream& GetOutputStream();

protected:
    explicit Environment(std::ostream& output = std::cout);
    explicit Environment(EnvironmentPtr outer);

    Environment(const Environment&) = delete;
    Environment &operator=(const Environment&) = delete;

protected:
    std::map<std::string, Value> m_values;
    Value m_returnValue;
    EnvironmentPtr m_outer = nullptr;
    bool m_break = false;
    bool m_return = false;
    std::ostream& m_outputStream; 
};

struct Interpreter : IExpressionVisitor, IStatementVisitor
{
    struct InterpreterError : std::exception // todo: move to cpp
    {
        InterpreterError(const Token& op, std::string message)
            : m_operator(op)
            , m_message(std::move(message))
        {}

        std::string m_message;
        const Token& m_operator;
    };

    Interpreter(EnvironmentPtr environment, std::map<const IExpression*, size_t>&& locals = std::map<const IExpression*, size_t>());
    void Interpret(EnvironmentPtr environment, const std::vector<IStatementPtr>& program, std::ostream& errorsLog) const;
    void Execute(const IStatement& statement, EnvironmentPtr environment) const;
protected:
    struct StatementVisitorContext : IStatementVisitorContext
    {
        explicit StatementVisitorContext(EnvironmentPtr environment)
            : m_environment(environment) {}

        EnvironmentPtr m_environment;
    };

    struct ExpressionVisitorContext : IExpressionVisitorContext 
    {
        ExpressionVisitorContext(EnvironmentPtr environment) : m_environment(environment) {} 
        EnvironmentPtr m_environment;
        Value m_result;
    };
    
    virtual void VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitFunctionDeclarationStatement(const FunctionDeclarationStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitIfStatement(const IfStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitWhileStatement(const WhileStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitBreakStatement(const BreakStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitReturnStatement(const ReturnStatement& statement, IStatementVisitorContext* context) const override;

    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLogicalExpression(const LogicalExpression& logicalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitCallExpression(const CallExpression& callExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLambdaExpression(const LambdaExpression& lambdaExpression, IExpressionVisitorContext* context) const override;


    Value Eval(const IExpression& expression, EnvironmentPtr environment) const;

    void RegisterNativeFunctions(EnvironmentPtr environment) const;

    static bool AreEqual(const Token& token, const Value& lhs, const Value& rhs);
    static double GetNumberOperand(const Token& token, const Value& lhs);

    static EnvironmentPtr GetEnvironment(IExpressionVisitorContext& context);
    static EnvironmentPtr GetEnvironment(IStatementVisitorContext& context);

    std::map<const IExpression*, size_t> m_locals;    
};
