#pragma once

#include "callable.h"

struct FunctionDeclarationStatement;

class Function : public ICallable
{
public:
    Function(const FunctionDeclarationStatement& declaration);

protected:
    virtual Value Call(const Interpreter& interpreter, Environment& globals, const std::vector<Value>& arguments) const override;

    virtual int Arity() const override;
    virtual std::string_view ToString() const override;
   
    const FunctionDeclarationStatement& m_declaration;
};