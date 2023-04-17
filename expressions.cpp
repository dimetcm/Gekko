#include "expressions.h"
#include "expressionvisitor.h"

UnaryExpression::UnaryExpression(const Token& op, IExpressionPtr expression)
    : m_expression(std::move(expression))
    , m_operator(op)
{}

void UnaryExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitUnaryExpression(*this, context);
}

BinaryExpression::BinaryExpression(IExpressionPtr left, const Token& op, IExpressionPtr right)
    : m_left(std::move(left))
    , m_operator(op)
    , m_right(std::move(right))
{}

void BinaryExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitBinaryExpression(*this, context);
}

TernaryConditionalExpression::TernaryConditionalExpression(IExpressionPtr condition, IExpressionPtr trueBranch, IExpressionPtr falseBranch)
    : m_condition(std::move(condition))
    , m_trueBranch(std::move(trueBranch))
    , m_falseBranch(std::move(falseBranch))
{}


void TernaryConditionalExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
       visitor.VisitTernaryConditionalExpression(*this, context);
}

GroupingExpression::GroupingExpression(IExpressionPtr expression)
    : m_expression(std::move(expression))
{}

void GroupingExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitGroupingExpression(*this, context);
}

LiteralExpression::LiteralExpression()
{}

LiteralExpression::LiteralExpression(std::any value)
    : m_value(value)
{}

void LiteralExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitLiteralExpression(*this, context);
}