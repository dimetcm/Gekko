#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "value.h"
//#include "statements.h"

struct Token;
struct IExpressionVisitor;
struct IExpressionVisitorContext;

class IExpression
{
public:
    virtual ~IExpression() {}
    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context = nullptr) const = 0;
};

using IExpressionPtr = std::unique_ptr<const IExpression>;

struct UnaryExpression : public IExpression
{
    UnaryExpression(const Token& op, IExpressionPtr expression);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_expression;
    const Token& m_operator;
};

struct BinaryExpression : public IExpression
{
    BinaryExpression(IExpressionPtr left, const Token& op, IExpressionPtr right);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_left;
    const Token& m_operator;
    IExpressionPtr m_right;
};

struct TernaryConditionalExpression : public IExpression
{
    TernaryConditionalExpression(IExpressionPtr condition, IExpressionPtr trueBranch, IExpressionPtr falseBranch);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_condition;
    IExpressionPtr m_trueBranch;
    IExpressionPtr m_falseBranch;
};

struct GroupingExpression : public IExpression
{
    explicit GroupingExpression(IExpressionPtr expression);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_expression;
};

struct LiteralExpression : public IExpression
{
    LiteralExpression();
    explicit LiteralExpression(Value value);
    
    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    Value m_value;
};

struct VariableExpression : public IExpression
{
    explicit VariableExpression(const Token& name);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_name;
};

struct AssignmentExpression : public IExpression
{
     AssignmentExpression(const Token& name, IExpressionPtr expression);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_name;
    IExpressionPtr m_expression;
};

struct LogicalExpression : public IExpression
{
    LogicalExpression(IExpressionPtr left, const Token& op, IExpressionPtr right);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    IExpressionPtr m_left;
    const Token& m_operator;
    IExpressionPtr m_right;
};

struct CallExpression : public IExpression
{
    CallExpression(IExpressionPtr calle, const Token& token, std::vector<IExpressionPtr>&& arguments);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    const Token& m_token;
    IExpressionPtr m_calle;
    std::vector<IExpressionPtr> m_arguments;
};

struct IStatement;
using IStatementPtr = std::unique_ptr<const IStatement>;

struct LambdaExpression : public IExpression
{
    using BodyType = std::vector<IStatementPtr>;
    using ParametersType = std::vector<std::reference_wrapper<const Token>>;

    LambdaExpression(ParametersType&& parameters, BodyType&& body);

    virtual void Accept(const IExpressionVisitor& visitor, IExpressionVisitorContext* context) const override;

    ParametersType m_parameters;
    BodyType m_body;
};