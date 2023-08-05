#pragma once

#include "statementvisitor.h"
#include "expressionvisitor.h"
#include <vector>
#include <map>
#include <memory>

struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

struct IExpression;
using IExpressiontPtr = std::unique_ptr<const IExpression>;

struct ResolverContext;
struct Token;

class Resolver : IExpressionVisitor, IStatementVisitor
{
public:
    struct Result
    {
        bool m_hasErrors = false;
        std::map<const IExpression*, size_t> m_locals; // resolved local names with distances to their declaration scope 
    };

    Result Resolve(const std::vector<IStatementPtr>& statements) const;
private:

    void Resolve(const IStatementPtr& statement, ResolverContext& context) const;
    void Resolve(const IExpression& expression, ResolverContext& context) const;
    void Resolve(const std::vector<IStatementPtr>& statements, ResolverContext& context) const;
    using FuncParametersType = std::vector<std::reference_wrapper<const Token>>;
    using FuncBodyType = std::vector<IStatementPtr>;
    void ResolveFunction(const FuncParametersType& params, const FuncBodyType& body,
                         ResolverContext& context, enum class FunctionType functionType) const;

    virtual void VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitFunctionDeclarationStatement(const FunctionDeclarationStatement& statement, IStatementVisitorContext* context) const override;
    virtual void VisitClassDeclarationStatement(const ClassDeclarationStatement& statement, IStatementVisitorContext* context) const override;
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
    virtual void VisitGetExpression(const GetExpression& getExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitSetExpression(const SetExpression& setExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLambdaExpression(const LambdaExpression& lambdaExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitThisExpression(const ThisExpression& thisExpression, IExpressionVisitorContext* context) const override;
};