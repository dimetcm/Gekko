#pragma once

struct IStatementVisitorContext
{
    virtual ~IStatementVisitorContext() {}
};

struct ExpressionStatement;
struct PrintStatement;
struct VariableDeclarationStatement;
struct BlockStatement;

struct IStatementVisitor
{
    virtual ~IStatementVisitor() {}

    virtual void VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const {}
};
