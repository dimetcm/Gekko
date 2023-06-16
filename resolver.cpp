#include "resolver.h"
#include "statements.h"
#include "expressions.h"
#include "token.h"
#include "gekko.h"
#include <assert.h>

struct ResolverContext : IStatementVisitorContext, IExpressionVisitorContext
{
    ResolverContext(std::map<const IExpression*, size_t>& locals)
        : m_locals(locals)
    {}
    
    enum class State
    {
        Declared,
        Defined
    };

    void BeginScope()
    {
        m_scopes.emplace_back();
    }

    void EndScope()
    {
        m_scopes.pop_back();
    }

    void Declare(const Token& name)
    {
        if (!m_scopes.empty())
        {
            Scope& scope = m_scopes.back();
            if (scope.contains(name.m_lexeme))
            {
                Gekko::ReportError(name, "Already a variable with this name in this scope.");
            }
            scope[name.m_lexeme] = State::Declared;
        }
    }

    void Define(const Token& name)
    {
        if (!m_scopes.empty())
        {
            m_scopes.back()[name.m_lexeme] = State::Defined;
        }
    }

    void ResolveLocal(const IExpression& expression, const Token& name)
    {
        for (size_t i = m_scopes.size(); i-- > 0;)
        {
            if (m_scopes[i].contains(name.m_lexeme))
            {
                m_locals[&expression] = m_scopes.size() - i - 1;
                return;
            }
        }
    }

    void OwnInitializerCheck(const Token& name) const
    {
        if (!m_scopes.empty())
        {
            const Scope& localScope = m_scopes.back();            
            auto it = localScope.find(name.m_lexeme);
            if (it != localScope.end())
            {
                if (it->second == State::Declared)
                {
                    Gekko::ReportError(name, "Can't read local variable in its own initializer.");
                }
            }
        }
    }

    using Scope = std::map<std::string_view, State>; 
    std::vector<Scope> m_scopes;

    std::map<const IExpression*, size_t>& m_locals;

    bool m_isInsideFunction = false;
    bool m_isInsideCycle = false;
};

ResolverContext& GetResolverContext(IStatementVisitorContext& context)
{
    return static_cast<ResolverContext&>(context);
}

ResolverContext& GetResolverContext(IExpressionVisitorContext& context)
{
    return static_cast<ResolverContext&>(context);
}

Resolver::Result Resolver::Resolve(const std::vector<IStatementPtr>& statements) const
{
    Resolver::Result result;

    ResolverContext context(result.m_locals);
    Resolve(statements, context);

    return result;
}

void Resolver::Resolve(const std::vector<IStatementPtr>& statements, ResolverContext& context) const
{
    for (const IStatementPtr& statement : statements)
    {
        Resolve(statement, context);
    }
}
   
void Resolver::Resolve(const IStatementPtr& statement, ResolverContext& context) const
{
    statement->Accept(*this, &context);
}

void Resolver::Resolve(const IExpressiontPtr& expression, ResolverContext& context) const
{
    expression->Accept(*this, &context);
}

void Resolver::ResolveFunction(const FuncParametersType& params, const FuncBodyType& body, ResolverContext& context) const
{
    context.BeginScope();

    for (const Token& parameter : params)
    {
        context.Declare(parameter);
        context.Define(parameter);
    }
    
    const bool oldIsInsideFunction = context.m_isInsideFunction;
    context.m_isInsideFunction = true;

    Resolve(body, context);

    context.m_isInsideFunction = oldIsInsideFunction;

    context.EndScope();    
}

void Resolver::VisitExpressionStatement(const ExpressionStatement& statement, IStatementVisitorContext* context) const
{
    Resolve(statement.m_expression, GetResolverContext(*context));
}

void Resolver::VisitPrintStatement(const PrintStatement& statement, IStatementVisitorContext* context) const
{
    Resolve(statement.m_expression, GetResolverContext(*context));
}

void Resolver::VisitVariableDeclarationStatement(const VariableDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);
    resolverContext.Declare(statement.m_name);
    if (statement.m_initializer)
    {
        Resolve(statement.m_initializer, resolverContext);
    }
    resolverContext.Define(statement.m_name);
}

void Resolver::VisitFunctionDeclarationStatement(const FunctionDeclarationStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context); 

    resolverContext.Declare(statement.m_name);
    resolverContext.Define(statement.m_name);
    
    ResolveFunction(statement.m_parameters, statement.m_body, resolverContext);
}

void Resolver::VisitBlockStatement(const BlockStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context); 

    resolverContext.BeginScope();
    Resolve(statement.m_block, resolverContext);
    resolverContext.EndScope();
}

void Resolver::VisitIfStatement(const IfStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context); 

    Resolve(statement.m_condition, resolverContext);
    Resolve(statement.m_trueBranch, resolverContext);
    if (statement.m_falseBranch)
    {
        Resolve(statement.m_falseBranch, resolverContext);
    }
}

void Resolver::VisitWhileStatement(const WhileStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(statement.m_condition, resolverContext);

    const bool oldIsInsideCycle = resolverContext.m_isInsideCycle;
    resolverContext.m_isInsideCycle = true;

    Resolve(statement.m_body, resolverContext);

    resolverContext.m_isInsideCycle = oldIsInsideCycle;
}

void Resolver::VisitBreakStatement(const BreakStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    if (!resolverContext.m_isInsideCycle) 
    {
        Gekko::ReportError(statement.m_keyword, "Break encountered outside of a cycle.");
    }
}

void Resolver::VisitReturnStatement(const ReturnStatement& statement, IStatementVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    if (!resolverContext.m_isInsideFunction) 
    {
        Gekko::ReportError(statement.m_keyword, "Can't return from top-level code.");
    }

    Resolve(statement.m_returnValue, resolverContext);
}

void Resolver::VisitUnaryExpression(const UnaryExpression& unaryExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(unaryExpression.m_expression, resolverContext);
}

void Resolver::VisitBinaryExpression(const BinaryExpression& binaryExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(binaryExpression.m_left, resolverContext);
    Resolve(binaryExpression.m_right, resolverContext);
}

void Resolver::VisitTernaryConditionalExpression(const TernaryConditionalExpression& ternaryConditionalExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(ternaryConditionalExpression.m_condition, resolverContext);
    Resolve(ternaryConditionalExpression.m_trueBranch, resolverContext);
    Resolve(ternaryConditionalExpression.m_falseBranch, resolverContext);
}

void Resolver::VisitGroupingExpression(const GroupingExpression& groupingExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(groupingExpression.m_expression, resolverContext); 
}

void Resolver::VisitLiteralExpression(const LiteralExpression& literalExpression, IExpressionVisitorContext* context) const
{}

void Resolver::VisitVariableExpression(const VariableExpression& variableExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);
    resolverContext.OwnInitializerCheck(variableExpression.m_name);
    resolverContext.ResolveLocal(variableExpression, variableExpression.m_name);
}

void Resolver::VisitAssignmentExpression(const AssignmentExpression& assignmentExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);
    Resolve(assignmentExpression.m_expression, resolverContext);
    resolverContext.ResolveLocal(assignmentExpression, assignmentExpression.m_name);
}

void Resolver::VisitLogicalExpression(const LogicalExpression& logicalExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(logicalExpression.m_left, resolverContext);
    Resolve(logicalExpression.m_right, resolverContext);
}

void Resolver::VisitCallExpression(const CallExpression& callExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    Resolve(callExpression.m_calle, resolverContext);

    for (const IExpressionPtr& argument : callExpression.m_arguments)
    {
        Resolve(argument, resolverContext);
    }
}

void Resolver::VisitLambdaExpression(const LambdaExpression& lambdaExpression, IExpressionVisitorContext* context) const
{
    ResolverContext& resolverContext = GetResolverContext(*context);

    ResolveFunction(lambdaExpression.m_parameters, lambdaExpression.m_body, resolverContext);
}