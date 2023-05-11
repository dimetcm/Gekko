#pragma once

#include <memory>
#include <vector>

struct Token;

struct IStatementVisitor;
struct IStatementVisitorContext;

struct IStatement
{
    virtual ~IStatement() {}
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const = 0;
};

using IStatementPtr = std::unique_ptr<const IStatement>;

class IExpression;
using IExpressionPtr = std::unique_ptr<const IExpression>;

struct ExpressionStatement : IStatement
{
    explicit ExpressionStatement(IExpressionPtr expression);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const;

    IExpressionPtr m_expression;
};

// temporary print statement, will be replaced with general FunctionStatement
struct PrintStatement : IStatement
{
    explicit PrintStatement(IExpressionPtr expression);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const;

    IExpressionPtr m_expression;
};

struct VariableDeclarationStatement : IStatement
{
    VariableDeclarationStatement(const Token& name, IExpressionPtr initializer);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const;

    const Token& m_name;
    IExpressionPtr m_initializer;
};

struct BlockStatement : IStatement
{
    explicit BlockStatement(std::vector<IStatementPtr>&& block);
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const;

    std::vector<IStatementPtr> m_block;
};

struct IfStatement : IStatement
{
    IfStatement(IExpressionPtr condition, IStatementPtr trueBranch, IStatementPtr falseBranch);
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const;

    IExpressionPtr m_condition;
    IStatementPtr m_trueBranch;
    IStatementPtr m_falseBranch;
};
