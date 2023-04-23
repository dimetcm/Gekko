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

struct IExpressionVisitor
{
    virtual ~IExpressionVisitor() {}
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const = 0;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const = 0;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const = 0;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const = 0;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const = 0;
    virtual void VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const = 0;
};
