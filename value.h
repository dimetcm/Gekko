#pragma once

#include <any>
#include <string>
#include <memory>

class ICallable;
class Class;
class ClassInstance;

class Value // todo: think about splitting Value class into those that hold by value or keep unique_ptr to value 
{
public:
    Value();
    explicit Value(bool value);
    explicit Value(double value);
    explicit Value(const std::string& value);
    explicit Value(const ICallable* value);
    explicit Value(std::shared_ptr<const Class> value);
    explicit Value(std::shared_ptr<const ClassInstance> value);

    const double* GetNumber() const;
    const std::string* GetString() const;
    const bool* GetBoolean() const;
    const ICallable* const* GetCallable() const;
    const std::shared_ptr<const Class>* GetClass() const;
    const std::shared_ptr<const ClassInstance>* GetClassInstace() const;

    bool IsTruthy() const;
    bool HasValue() const;
    bool IsNil() const;

    std::string ToString() const;
private:
    std::any m_value;
};

