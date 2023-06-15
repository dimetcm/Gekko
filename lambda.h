#pragma once

#include "callable.h"

struct LambdaExpression;

class Lambda : public ICallable
{
public:
    Lambda(const LambdaExpression& lambdaExpression, EnvironmentPtr closure);

protected:
    virtual Value Call(const Interpreter& interpreter, EnvironmentPtr globals, FunctionsRegistry& functionsRegistry, const std::vector<Value>& arguments) const override;
    virtual int Arity() const override;
    virtual std::string ToString() const override;

protected:
    const LambdaExpression& m_lambdaExpression;
    EnvironmentPtr m_closure;
};