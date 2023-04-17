#pragma once

#include <any>
#include "expressionvisitor.h"

struct Interpreter : IExpressionVisitor 
{
    struct Context : IExpressionVisitorContext 
    {
        std::any m_result;
    };
    
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override;
};
