#pragma once

#include <any>

struct Token;

struct UnaryExpression;
struct BinaryExpression;
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
        virtual void VisitGroupingExpression(const GroupingExpression& groupingExpression, IContext* context) const = 0;
        virtual void VisitLiteralExpression(const LiteralExpression& literalExpression, IContext* context) const = 0;
    };

    virtual ~IExpression() {}
    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context = nullptr) const = 0;
};

struct UnaryExpression : public IExpression
{
    UnaryExpression(const Token& op, const IExpression& expression);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    const IExpression& m_expression;
    const Token& m_operator;
};

struct BinaryExpression : public IExpression
{
    BinaryExpression(const IExpression& left, const Token& op, const IExpression& right);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    const IExpression& m_left;
    const Token& m_operator;
    const IExpression& m_right;
};

struct GroupingExpression : public IExpression
{
    GroupingExpression(const IExpression& expression);

    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    const IExpression& m_expression;
};

struct LiteralExpression : public IExpression
{
    LiteralExpression(const std::any& value);
    
    virtual void Accept(const IVisitor& visitor, IVisitor::IContext* context) const override;

    const std::any& m_value;
};

