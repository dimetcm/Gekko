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
