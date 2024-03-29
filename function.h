#pragma once

#include "callable.h"

struct FunctionDeclarationStatement;

class Function : public ICallable
{
public:
    Function(const FunctionDeclarationStatement& declaration, EnvironmentPtr closure);
    const Function* Bind(std::shared_ptr<ClassInstance> classInstance, FunctionsRegistry& functionsRegistry) const;
    virtual Value Call(const Interpreter& interpreter, EnvironmentPtr globals, FunctionsRegistry& functionsRegistry, const std::vector<Value>& arguments) const override;
    virtual int Arity() const override;
    virtual std::string ToString() const override;
  
protected:
    const FunctionDeclarationStatement& m_declaration;
    EnvironmentPtr m_closure;
};