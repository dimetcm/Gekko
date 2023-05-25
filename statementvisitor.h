#pragma once

struct IStatementVisitorContext
{
    virtual ~IStatementVisitorContext() {}
};

struct ExpressionStatement;
struct PrintStatement;
struct VariableDeclarationStatement;
struct FunctionDeclarationStatement;
struct BlockStatement;
struct IfStatement;
struct WhileStatement;
struct BreakStatement;
struct ReturnStatement;

struct IStatementVisitor
{
    virtual ~IStatementVisitor() {}

    virtual void VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitFunctionDeclarationStatement(const FunctionDeclarationStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitIfStatement(const IfStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitWhileStatement(const WhileStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitBreakStatement(const BreakStatement& statement, IStatementVisitorContext* context) const {}
    virtual void VisitReturnStatement(const ReturnStatement& statement, IStatementVisitorContext* context) const {}
};
