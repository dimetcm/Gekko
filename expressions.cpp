#include "expressions.h"

UnaryExpression::UnaryExpression(const Token& op, const IExpression& expression)
    : m_expression(expression)
    , m_operator(op)
{}

void UnaryExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitUnaryExpression(*this, context);
}

BinaryExpression::BinaryExpression(const IExpression& left, const Token& op, const IExpression& right)
    : m_left(left)
    , m_operator(op)
    , m_right(right)
{}

void BinaryExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitBinaryExpression(*this, context);
}

GroupingExpression::GroupingExpression(const IExpression& expression)
    : m_expression(expression)
{}

void GroupingExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitGroupingExpression(*this, context);
}

LiteralExpression::LiteralExpression(const std::any& value)
    : m_value(value)
{}

void LiteralExpression::Accept(const IVisitor& visitor, IVisitor::IContext* context) const
{
    visitor.VisitLiteralExpression(*this, context);
}