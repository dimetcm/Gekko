#pragma once

#include <any>
#include <string>

class Value
{
public:
    Value();
    explicit Value(bool value);
    explicit Value(double value);
    explicit Value(const std::string& value);

    const double* GetNumber() const;
    const std::string* GetString() const;
    const bool* GetBoolean() const;

    bool IsTruthy() const;
    bool HasValue() const;

    std::string ToString() const;
private:
    std::any m_value;
};

