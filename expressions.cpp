#include "expressions.h"

UnaryExpression::UnaryExpression(const Token& op, IExpressionPtr expression)
    : m_expression(std::move(expression))
    , m_operator(op)
{}

void UnaryExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitUnaryExpression(*this, context);
}

BinaryExpression::BinaryExpression(IExpressionPtr left, const Token& op, IExpressionPtr right)
    : m_left(std::move(left))
    , m_operator(op)
    , m_right(std::move(right))
{}

void BinaryExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitBinaryExpression(*this, context);
}

GroupingExpression::GroupingExpression(IExpressionPtr expression)
    : m_expression(std::move(expression))
{}

void GroupingExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitGroupingExpression(*this, context);
}

LiteralExpression::LiteralExpression(std::any value)
    : m_value(value)
{}

void LiteralExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitLiteralExpression(*this, context);
}