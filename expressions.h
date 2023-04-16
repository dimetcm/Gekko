#pragma once

#include <any>
#include <memory>

struct Token;

struct UnaryExpression;
struct BinaryExpression;
struct TernaryConditionalExpression;
struct GroupingExpression;
struct LiteralExpression;


class IExpression
{
public:
    struct IVisitor
    {
        struct IContext
        {
            virtual ~IContext() {}
        };

        virtual ~IVisitor() {}
        virtual void VisitUnaryExpression(const UnaryExpression& unaryExpression, IContext* context) const = 0;
        virtual void VisitBinaryExpression(const BinaryExpression& binaryExpression, IContext* context) const = 0;
        virtual void VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IContext* context) const = 0;
        virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IContext* context) const = 0;
        virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IContext* context) const = 0;
    };

    virtual ~IExpression() {}
    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context = nullptr) const = 0;
};

using IExpressionPtr = std::unique_ptr<const IExpression>;

struct UnaryExpression : public IExpression
{
    UnaryExpression(const Token& op, IExpressionPtr expression);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    IExpressionPtr m_expression;
    const Token& m_operator;
};

struct BinaryExpression : public IExpression
{
    BinaryExpression(IExpressionPtr left, const Token& op, IExpressionPtr right);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    IExpressionPtr m_left;
    const Token& m_operator;
    IExpressionPtr m_right;
};

struct TernaryConditionalExpression : public IExpression
{
    TernaryConditionalExpression(IExpressionPtr condition, IExpressionPtr trueBranch, IExpressionPtr falseBranch);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    IExpressionPtr m_condition;
    IExpressionPtr m_trueBranch;
    IExpressionPtr m_falseBranch;
};

struct GroupingExpression : public IExpression
{
    explicit GroupingExpression(IExpressionPtr expression);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    IExpressionPtr m_expression;
};

struct LiteralExpression : public IExpression
{
    explicit LiteralExpression(std::any value);
    
    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    std::any m_value;
};

