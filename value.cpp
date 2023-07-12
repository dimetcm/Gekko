#include "value.h"
#include "callable.h"
#include "class.h"

Value::Value()
{}

Value::Value(bool value)
    : m_value(std::make_any<bool>(value))
{}

Value::Value(double value)
    : m_value(std::make_any<double>(value))
{}


Value::Value(const std::string& value)
    : m_value(std::make_any<std::string>(value))
{}

Value::Value(const ICallable* value)
    : m_value(std::make_any<const ICallable*>(value))
{}

Value::Value(std::shared_ptr<const Class> value)
    : m_value(std::make_any<std::shared_ptr<const Class>>(value))
{}

const double* Value::GetNumber() const
{
    return std::any_cast<double>(&m_value);
}

const std::string* Value::GetString() const
{
    return std::any_cast<std::string>(&m_value);
}

const ICallable* const* Value::GetCallable() const
{
    return std::any_cast<const ICallable*>(&m_value);
}

const std::shared_ptr<const Class>* Value::GetClass() const
{
    return std::any_cast<const std::shared_ptr<const Class>>(&m_value);
}

bool Value::IsTruthy() const
{
    if (!m_value.has_value())
    {
        return false;
    }
    else if (const bool* result = std::any_cast<bool>(&m_value))
    {
        return *result;
    }

    return true;
}

const bool* Value::GetBoolean() const
{
    return std::any_cast<bool>(&m_value);
}

std::string Value::ToString() const
{
    if (!m_value.has_value())
    {
        return "nil";
    }
    else if (const double* value = GetNumber())
    {
        return std::to_string(*value);
    }
    else if (const std::string* value = GetString())
    {
        return *value;
    }
    else if (const bool* value = GetBoolean())
    {
        return *value ? "true" : "false";
    }
    else if (const ICallable* const* value = GetCallable())
    {
        return std::string((*value)->ToString());
    }
    else if (const std::shared_ptr<const Class>* value = GetClass())
    {
        return std::string((*value)->ToString());        
    }

    return "Unsupported value type";
}

bool Value::HasValue() const
{
    return m_value.has_value();
}

bool Value::IsNil() const
{
    return !HasValue();
}