#pragma once

#include "../interpreter.h"

struct MockedEnvironment : Interpreter::Environment
{
    bool Hasvalue(const std::string& name) const { return m_values.find(name) != m_values.end(); }
    Value GetValue(const std::string& name) const { return m_values.find(name)->second; }
};

struct MockedInterpreter : Interpreter
{
    void Execute(const IStatement& statement, Environment& environment) const
    {
        Interpreter::Execute(statement, environment);
    }

    Value Eval(const IExpression& expression, Environment& environment) const
    {
        return Interpreter::Eval(expression, environment);
    }
};

