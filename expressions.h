#pragma once

#include <any>

struct Token;
struct IExpressionVisitor;

class IExpression
{
public:
    virtual ~IExpression() {}
    virtual void Accept(IExpressionVisitor& visitor) const = 0;
};

struct UnaryExpression : public IExpression
{
    UnaryExpression(const IExpression& expression, const Token& op);

    virtual void Accept(IExpressionVisitor& visitor) const override;

    const IExpression& m_expression;
    const Token& m_operator;
};

struct BinaryExpression : public IExpression
{
    BinaryExpression(const IExpression& left, const Token& op, const IExpression& right);

    virtual void Accept(IExpressionVisitor& visitor) const override;

    const IExpression& m_left;
    const Token& m_operator;
    const IExpression& m_right;
};

struct GroupingExpression : public IExpression
{
    GroupingExpression(const IExpression& expression);

    virtual void Accept(IExpressionVisitor& visitor) const override;

    const IExpression& m_expression;
};

struct LiteralExpression : public IExpression
{
    LiteralExpression(const std::any& value);
    
    virtual void Accept(IExpressionVisitor& visitor) const override;

    const std::any& m_value;
};

struct IExpressionVisitor
{
    virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression) = 0;
    virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression) = 0;
    virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression) = 0;
    virtual void VisitLiteralExpression(const LiteralExpression& literalExpression) = 0;
};
