#include "expressions.h"

UnaryExpression::UnaryExpression(const IExpression& expression, const Token& op)
    : m_expression(expression)
    , m_operator(op)
{}

void UnaryExpression::Accept(IExpressionVisitor& visitor) const
{
    visitor.VisitUnaryExpression(*this);
}

BinaryExpression::BinaryExpression(const IExpression& left, const Token& op, const IExpression& right)
    : m_left(left)
    , m_operator(op)
    , m_right(right)
{}

void BinaryExpression::Accept(IExpressionVisitor& visitor) const
{
    visitor.VisitBinaryExpression(*this);
}

GroupingExpression::GroupingExpression(const IExpression& expression)
    : m_expression(expression)
{}

void GroupingExpression::Accept(IExpressionVisitor& visitor) const
{
    visitor.VisitGroupingExpression(*this);
}

LiteralExpression::LiteralExpression(const std::any& value)
    : m_value(value)
{}

void LiteralExpression::Accept(IExpressionVisitor& visitor) const
{
    visitor.VisitLiteralExpression(*this);
}