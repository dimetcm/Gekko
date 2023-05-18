#pragma once

#include "value.h"
#include <vector>

class ICallable
{
public:
    virtual ~ICallable() {}
    virtual Value Call(const std::vector<Value>& arguments) const = 0;
    virtual int Arity() const = 0;
    virtual std::string ToString() const = 0;
};