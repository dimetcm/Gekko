#pragma once

#include <string>
#include "expressions.h"
#include "expressionvisitor.h"
#include "token.h"


struct ASTPrinter : IExpressionVisitor
{
    struct VisitorContext : IExpressionVisitorContext 
    {
        std::string m_result;
    };

    std::string ToString(const IExpression& expression) const
    {
        VisitorContext context;
        expression.Accept(*this, &context);
        return context.m_result;
    }

private:
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const override
    {
        Parenthesize(*static_cast<VisitorContext*>(context), unaryExpression.m_operator.m_lexeme, {&*unaryExpression.m_expression});
    }

    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const  override
    {
        Parenthesize(*static_cast<VisitorContext*>(context), binaryExpression.m_operator.m_lexeme, {&*binaryExpression.m_left, &*binaryExpression.m_right});        
    }

    virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const override
    {
        VisitorContext* visitorContext = static_cast<VisitorContext*>(context);

        ternaryConditionalExpression.m_condition->Accept(*this, context);
        visitorContext->m_result.append("?");
        ternaryConditionalExpression.m_trueBranch->Accept(*this, context);
        visitorContext->m_result.append(":");
        ternaryConditionalExpression.m_falseBranch->Accept(*this, context);
    }

    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const override
    {
        Parenthesize(*static_cast<VisitorContext*>(context), "group", {&*groupingExpression.m_expression});
    }

    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const override
    {
        VisitorContext* visitorContext = static_cast<VisitorContext*>(context);

        visitorContext->m_result.append(literalExpression.m_value.ToString());
    }

    void Parenthesize(VisitorContext& context, std::string_view name, std::initializer_list<const IExpression*> expressions) const
    {
        context.m_result.append("(").append(name);

        for (const IExpression* expression : expressions)
        {
            context.m_result.append(" ");
            expression->Accept(*this, &context);
        }

        context.m_result.append(")");
    }
};