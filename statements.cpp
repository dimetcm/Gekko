#include "statements.h"
#include "expressions.h"
#include "statementvisitor.h"

ExpressionStatement::ExpressionStatement(IExpressionPtr expression)
    : m_expression(std::move(expression))
{}

void ExpressionStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitExpressionStatement(*this, context);
}

PrintStatement::PrintStatement(IExpressionPtr expression)
    : m_expression(std::move(expression))
{}

void PrintStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitPrintStatement(*this, context);
}

VariableDeclarationStatement::VariableDeclarationStatement(const Token& name, IExpressionPtr initializer)
    : m_name(name)
    , m_initializer(std::move(initializer))
{}

void VariableDeclarationStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitVariableDeclarationStatement(*this, context);
}

FunctionDeclarationStatement::FunctionDeclarationStatement(
    const Token& name, ParametersType&& parameters, BodyType&& body)
    : m_name(name)
    , m_parameters(std::move(parameters))
    , m_body(std::move(body))
{}

void FunctionDeclarationStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitFunctionDeclarationStatement(*this, context);
}

ClassDeclarationStatement::ClassDeclarationStatement(const Token& name, std::vector<IStatementPtr>&& methods)
    : m_name(name)
    , m_methods(std::move(methods))
{}

void ClassDeclarationStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitClassDeclarationStatement(*this, context);
}

BlockStatement::BlockStatement(std::vector<IStatementPtr>&& block)
    : m_block(std::move(block))
{}

void BlockStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitBlockStatement(*this, context);
}

IfStatement::IfStatement(IExpressionPtr condition, IStatementPtr trueBranch, IStatementPtr falseBranch)
    : m_condition(std::move(condition))
    , m_trueBranch(std::move(trueBranch))
    , m_falseBranch(std::move(falseBranch))
{}

void IfStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitIfStatement(*this, context);    
}

WhileStatement::WhileStatement(IExpressionPtr condition, IStatementPtr body)
    : m_condition(std::move(condition))
    , m_body(std::move(body))
{}

void WhileStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitWhileStatement(*this, context);    
}

BreakStatement::BreakStatement(const Token& keyword)
    : m_keyword(keyword)
{}

void BreakStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitBreakStatement(*this, context);
}

ReturnStatement::ReturnStatement(IExpressionPtr returnValue, const Token& keyword)
    : m_returnValue(std::move(returnValue))
    , m_keyword(keyword)
{}

void ReturnStatement::Accept(const IStatementVisitor& visitor, IStatementVisitorContext* context) const
{
    visitor.VisitReturnStatement(*this, context);
}