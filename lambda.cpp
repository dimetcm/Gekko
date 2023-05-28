#include "lambda.h"
#include "interpreter.h"
#include "expressions.h"
#include "token.h"
#include <assert.h>

Lambda::Lambda(const LambdaExpression& lambdaExpression, EnvironmentPtr closure)
    : m_lambdaExpression(lambdaExpression)
    , m_closure(closure)
{}

Value Lambda::Call(const Interpreter& interpreter, EnvironmentPtr globals, const std::vector<Value>& arguments) const
{
    assert(arguments.size() == m_lambdaExpression.m_parameters.size());

    EnvironmentPtr localEnvironment = Environment::CreateLocalEnvironment(m_closure);

    for (size_t i = 0; i < arguments.size(); ++i)
    {
        const Token& token = m_lambdaExpression.m_parameters[i];
        localEnvironment->Define(token.m_lexeme, arguments[i]);
    }

    for (const IStatementPtr& statement : m_lambdaExpression.m_body)
    {
        interpreter.Execute(*statement, localEnvironment);
        if (localEnvironment->ReturnRequested())
        {
            return localEnvironment->GetReturnValue();
        }
    }

    return Value();
}

int Lambda::Arity() const
{
    return static_cast<int>(m_lambdaExpression.m_parameters.size());
}

std::string Lambda::ToString() const
{
    return "<fn lambda>";
}
