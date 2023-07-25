#include "expressions.h"
#include "expressionvisitor.h"
#include "statements.h"

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

LiteralExpression::LiteralExpression(Value value)
    : m_value(value)
{}

void LiteralExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitLiteralExpression(*this, context);
}

VariableExpression::VariableExpression(const Token& name)
    : m_name(name)
{}

void VariableExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitVariableExpression(*this, context);
}

AssignmentExpression::AssignmentExpression(const Token& name, IExpressionPtr expression)
    : m_name(name)
    , m_expression(std::move(expression))
{}

void AssignmentExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitAssignmentExpression(*this, context);
}

LogicalExpression::LogicalExpression(IExpressionPtr left, const Token& op, IExpressionPtr right)
    : m_left(std::move(left))
    , m_operator(op)
    , m_right(std::move(right))
{}

void LogicalExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitLogicalExpression(*this, context);
}

CallExpression::CallExpression(IExpressionPtr calle, const Token& token, std::vector<IExpressionPtr>&& arguments)
    : m_calle(std::move(calle))
    , m_token(token)
    , m_arguments(std::move(arguments))
{
}

void CallExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitCallExpression(*this, context);
}

GetExpression::GetExpression(IExpressionPtr owner, const Token& name)
    : m_owner(std::move(owner))
    , m_name(name)
{}

void GetExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitGetExpression(*this, context);
}

LambdaExpression::LambdaExpression(ParametersType&& parameters, BodyType&& body)
    : m_parameters(std::move(parameters))
    , m_body(std::move(body))
{}

void LambdaExpression::Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const
{
    visitor.VisitLambdaExpression(*this, context);
}
