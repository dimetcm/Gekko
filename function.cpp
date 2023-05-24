#include "function.h"
#include "statements.h"
#include "token.h"
#include "interpreter.h"
#include <assert.h>
#include <algorithm>

Function::Function(const FunctionDeclarationStatement& declaration)
    : m_declaration(declaration)
{}

Value Function::Call(const Interpreter& interpreter, Environment& globalEnvironment, const std::vector<Value>& arguments) const
{
    assert(arguments.size() == m_declaration.m_parameters.size());

    Environment localEnvironment(globalEnvironment);

    for (size_t i = 0; i < arguments.size(); ++i)
    {
        const Token& token = m_declaration.m_parameters[i];
        localEnvironment.Define(token.m_lexeme, arguments[i]);

    }

    for (const IStatementPtr& statement : m_declaration.m_body)
    {
        interpreter.Execute(*statement, localEnvironment);        
    }

    return Value();
}
    
int Function::Arity() const
{
    return static_cast<int>(m_declaration.m_parameters.size());
}

std::string_view Function::ToString() const
{
    return "<fn " + std::string(m_declaration.m_name.m_lexeme) + ">"; 
}
