#pragma once

#include "Callable.h"
#include <string_view>

class Class;

class ClassInstance
{
public:
    ClassInstance(const Class& definition) : m_definition(definition) {}

    const Class& ClassDefinition() const { return m_definition; }
private:
    const Class& m_definition;
};

class Class
{
public:
    explicit Class(std::string_view name) : m_name(name) {}

    Value CreateInstance() const
    {
        std::shared_ptr<ClassInstance> instance = std::make_shared<ClassInstance>(*this);
        return Value(instance);
    }

    std::string_view ToString() const { return m_name; }

private:
    std::string_view m_name;
};