#pragma once

#include "value.h"
#include <vector>

struct Interpreter;
struct Environment;
using EnvironmentPtr = std::shared_ptr<Environment>; 
struct FunctionsRegistry;

class ICallable
{
public:
    virtual ~ICallable() {}

    virtual Value Call(const Interpreter& interpreter, EnvironmentPtr globals, FunctionsRegistry& functionsRegistry, const std::vector<Value>& arguments) const = 0;

    virtual int Arity() const = 0;
    virtual std::string ToString() const = 0;
};