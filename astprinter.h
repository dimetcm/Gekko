#pragma once

#include <string>
#include "expressions.h"


struct ASTPrinter : IExpression::IVisitor
{
    struct VisitorContext : IExpression::IVisitor::IContext 
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
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IContext* context) const override
    {
        const IExpression& child = *unaryExpression.m_expression;

        Parenthesize(*context, unaryExpression.m_operator.m_lexeme, {&*unaryExpression.m_expression});
    }

    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IContext* context) const  override
    {
        Parenthesize(*context, binaryExpression.m_operator.m_lexeme, {&*binaryExpression.m_left, &*binaryExpression.m_right});        
    }

    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IContext* context) const override
    {
        Parenthesize(*context, "group", {&*groupingExpression.m_expression});
    }

    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IContext* context) const override
    {
        VisitorContext* visitorContext = static_cast<VisitorContext*>(context);

        if (!literalExpression.m_value.has_value())
        {
            visitorContext->m_result.append("nil");
        }
        else  if (const double* value = std::any_cast<double>(&literalExpression.m_value))
        {
            visitorContext->m_result.append(std::to_string(*value));
        }
        else if (const std::string_view* value = std::any_cast<std::string_view>(&literalExpression.m_value))
        {
            visitorContext->m_result.append(*value);
        }
    }

    void Parenthesize(IContext& context, std::string_view name, std::initializer_list<const IExpression*> expressions) const
    {
        VisitorContext& visitorContext = static_cast<VisitorContext&>(context);
        visitorContext.m_result.append("(").append(name);

        for (const IExpression* expression : expressions)
        {
            visitorContext.m_result.append(" ");
            expression->Accept(*this, &context);
        }

        visitorContext.m_result.append(")");
    }
};