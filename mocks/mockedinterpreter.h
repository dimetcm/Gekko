#pragma once

#include "../interpreter.h"

struct MockedEnvironment;
using MockedEnvironmentPtr = std::shared_ptr<MockedEnvironment>;

struct MockedEnvironment : Environment
{
    static MockedEnvironmentPtr Create() { return std::shared_ptr<MockedEnvironment>(new MockedEnvironment()); }

    bool Hasvalue(const std::string& name) const { return m_values.find(name) != m_values.end(); }
    Value GetValue(const std::string& name) const { return m_values.find(name)->second; }
};

struct MockedInterpreter : Interpreter
{
    MockedInterpreter(EnvironmentPtr environment) : Interpreter(environment) {}
    
    void Execute(const IStatement& statement, EnvironmentPtr environment) const
    {
        Interpreter::Execute(statement, environment);
    }

    Value Eval(const IExpression& expression, EnvironmentPtr environment) const
    {
        return Interpreter::Eval(expression, environment);
    }
};

