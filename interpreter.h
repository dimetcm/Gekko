#pragma once

#include <any>
#include <iostream>
#include "expressionvisitor.h"
#include "value.h"

class IExpression;

struct Interpreter : IExpressionVisitor 
{
    Value Interpret(const IExpression& expression, std::ostream& logOutput) const; 
private:
    struct Context : IExpressionVisitorContext 
    {
        Value m_result;
    };
    
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override;

    Value Eval(const IExpression& expression) const;
};
