#include "function.h"
#include "statements.h"
#include "token.h"
#include "interpreter.h"
#include <assert.h>
#include <algorithm>

Function::Function(const FunctionDeclarationStatement& declaration, EnvironmentPtr closure)
    : m_declaration(declaration)
    , m_closure(closure)
{}

const Function* Function::Bind(std::shared_ptr<ClassInstance> classInstance, FunctionsRegistry& functionsRegistry) const
{
    EnvironmentPtr localEnvironment = Environment::CreateLocalEnvironment(m_closure);
    localEnvironment->Define(TokenTypeToStringView(Token::Type::This), Value(classInstance));
    return functionsRegistry.Register<Function>(m_declaration, localEnvironment);
}

Value Function::Call(const Interpreter& interpreter, EnvironmentPtr globalEnvironment, FunctionsRegistry& functionsRegistry, const std::vector<Value>& arguments) const
{
    assert(arguments.size() == m_declaration.m_parameters.size());

    EnvironmentPtr localEnvironment = Environment::CreateLocalEnvironment(m_closure);

    for (size_t i = 0; i < arguments.size(); ++i)
    {
        const Token& token = m_declaration.m_parameters[i];
        localEnvironment->Define(token.m_lexeme, arguments[i]);
    }

    for (const IStatementPtr& statement : m_declaration.m_body)
    {
        interpreter.Execute(*statement, localEnvironment, functionsRegistry);
        if (localEnvironment->ReturnRequested())
        {
            return localEnvironment->GetReturnValue();
        }
    }

    return Value();
}
    
int Function::Arity() const
{
    return static_cast<int>(m_declaration.m_parameters.size());
}

std::string Function::ToString() const
{
    return "<fn " + std::string(m_declaration.m_name.m_lexeme) + ">"; 
}
