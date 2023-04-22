#pragma once

struct IStatementVisitorContext
{
    virtual ~IStatementVisitorContext() {}
};

struct ExpressionStatement;
struct PrintStatement;
struct VariableDeclarationStatement;

struct IStatementVisitor
{
    virtual ~IStatementVisitor() {}

    virtual void VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const = 0;
    virtual void VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const = 0;
    virtual void VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const = 0;
};
