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

struct IExpression;
using IExpressionPtr = std::unique_ptr<const IExpression>;

struct ExpressionStatement : IStatement
{
    explicit ExpressionStatement(IExpressionPtr expression);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    IExpressionPtr m_expression;
};

// temporary print statement, will be replaced with general FunctionStatement
struct PrintStatement : IStatement
{
    explicit PrintStatement(IExpressionPtr expression);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    IExpressionPtr m_expression;
};

struct VariableDeclarationStatement : IStatement
{
    VariableDeclarationStatement(const Token& name, IExpressionPtr initializer);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    const Token& m_name;
    IExpressionPtr m_initializer;
};

struct FunctionDeclarationStatement : IStatement
{
    using ParametersType = std::vector<std::reference_wrapper<const Token>>;
    using BodyType = std::vector<IStatementPtr>;

    enum class FunctionDeclarationType
    {
        FreeFunction,
        MemberFunction,
        MemberStaticFunction,
        MemberGetter
    };

    FunctionDeclarationStatement(const Token& name, ParametersType&& parameters, BodyType&& body, FunctionDeclarationType type);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;


    const Token& m_name;
    ParametersType m_parameters;
    BodyType m_body;
    FunctionDeclarationType m_type;
};

struct ClassDeclarationStatement : IStatement
{
    ClassDeclarationStatement(const Token& name, std::vector<std::unique_ptr<FunctionDeclarationStatement>>&& methods);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    const Token& m_name;
    std::vector<std::unique_ptr<FunctionDeclarationStatement>> m_methods;
};

struct BlockStatement : IStatement
{
    explicit BlockStatement(std::vector<IStatementPtr>&& block);
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    std::vector<IStatementPtr> m_block;
};

struct IfStatement : IStatement
{
    IfStatement(IExpressionPtr condition, IStatementPtr trueBranch, IStatementPtr falseBranch);
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    IExpressionPtr m_condition;
    IStatementPtr m_trueBranch;
    IStatementPtr m_falseBranch;
};

struct WhileStatement : IStatement
{
    WhileStatement(IExpressionPtr condition, IStatementPtr body);
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    IExpressionPtr m_condition;
    IStatementPtr m_body;
};

struct BreakStatement : IStatement
{
    explicit BreakStatement(const Token& keyword);
    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    const Token& m_keyword;
};

struct ReturnStatement : IStatement
{
    ReturnStatement(IExpressionPtr returnValue, const Token& keyword);

    virtual void Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const override;

    IExpressionPtr m_returnValue;
    const Token& m_keyword;
};