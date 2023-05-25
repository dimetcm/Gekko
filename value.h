#pragma once

#include <any>
#include <string>
#include <memory>

class ICallable;

class Value
{
public:
    Value();
    explicit Value(bool value);
    explicit Value(double value);
    explicit Value(const std::string& value);
    explicit Value(std::shared_ptr<const ICallable> value);

    const double* GetNumber() const;
    const std::string* GetString() const;
    const bool* GetBoolean() const;
    const ICallable* GetCallable() const;

    bool IsTruthy() const;
    bool HasValue() const;
    bool IsNil() const;

    std::string ToString() const;
private:
    std::any m_value;
};

