#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "value.h"

struct Token;
struct IExpressionVisitor;
struct IExpressionVisitorContext;

struct IExpression
{
    virtual ~IExpression() {}
    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context = nullptr) const = 0;
};

using IExpressionPtr = std::unique_ptr<const IExpression>;

struct UnaryExpression : IExpression
{
    UnaryExpression(const Token& op, IExpressionPtr expression);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_expression;
    const Token& m_operator;
};

struct BinaryExpression : IExpression
{
    BinaryExpression(IExpressionPtr left, const Token& op, IExpressionPtr right);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_left;
    const Token& m_operator;
    IExpressionPtr m_right;
};

struct TernaryConditionalExpression : IExpression
{
    TernaryConditionalExpression(IExpressionPtr condition, IExpressionPtr trueBranch, IExpressionPtr falseBranch);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_condition;
    IExpressionPtr m_trueBranch;
    IExpressionPtr m_falseBranch;
};

struct GroupingExpression : IExpression
{
    explicit GroupingExpression(IExpressionPtr expression);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_expression;
};

struct LiteralExpression : IExpression
{
    LiteralExpression();
    explicit LiteralExpression(Value value);
    
    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    Value m_value;
};

struct VariableExpression : IExpression
{
    explicit VariableExpression(const Token& name);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_name;
};

struct AssignmentExpression : IExpression
{
     AssignmentExpression(const Token& name, IExpressionPtr expression);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_name;
    IExpressionPtr m_expression;
};

struct LogicalExpression : IExpression
{
    LogicalExpression(IExpressionPtr left, const Token& op, IExpressionPtr right);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_left;
    const Token& m_operator;
    IExpressionPtr m_right;
};

struct CallExpression : IExpression
{
    CallExpression(IExpressionPtr calle, const Token& token, std::vector<IExpressionPtr>&& arguments);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_token;
    IExpressionPtr m_calle;
    std::vector<IExpressionPtr> m_arguments;
};

struct GetExpression : IExpression
{
    GetExpression(IExpressionPtr owner, const Token& name);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_name;
    IExpressionPtr m_owner;
};

struct SetExpression : IExpression
{
    SetExpression(IExpressionPtr getter, const IExpression& owner, const Token& name, IExpressionPtr value);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_name;
    IExpressionPtr m_getter;
    IExpressionPtr m_value;
    const IExpression& m_owner;
};

struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

struct LambdaExpression : IExpression
{
    using BodyType = std::vector<IStatementPtr>;
    using ParametersType = std::vector<std::reference_wrapper<const Token>>;

    LambdaExpression(ParametersType&& parameters, BodyType&& body);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    ParametersType m_parameters;
    BodyType m_body;
};

struct ThisExpression : IExpression
{
    explicit ThisExpression(const Token& keyword);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_keyword;
};

struct SuperExpression : IExpression
{
    SuperExpression(const Token& keyword, const Token& method);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;
    
    const Token& m_keyword;
    const Token& m_method;
};