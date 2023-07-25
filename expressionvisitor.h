#pragma once

struct IExpressionVisitorContext
{
    virtual ~IExpressionVisitorContext() {}
};

struct UnaryExpression;
struct BinaryExpression;
struct TernaryConditionalExpression;
struct GroupingExpression;
struct LiteralExpression;
struct VariableExpression;
struct AssignmentExpression;
struct LogicalExpression;
struct CallExpression;
struct GetExpression;
struct LambdaExpression;

struct IExpressionVisitor
{
    virtual ~IExpressionVisitor() {}
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitLogicalExpression(const LogicalExpression& logicalExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitCallExpression(const CallExpression& callExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitGetExpression(const GetExpression& getExpression, IExpressionVisitorContext* context) const {}
    virtual void VisitLambdaExpression(const LambdaExpression& lambdaExpression, IExpressionVisitorContext* context) const {}
};
