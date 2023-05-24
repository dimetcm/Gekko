#pragma once

#include "value.h"
#include <vector>

struct Interpreter;
struct Environment;

class ICallable
{
public:
    virtual ~ICallable() {}

    virtual Value Call(const Interpreter& interpreter, Environment& globals, const std::vector<Value>& arguments) const = 0;

    virtual int Arity() const = 0;
    virtual std::string_view ToString() const = 0;
};