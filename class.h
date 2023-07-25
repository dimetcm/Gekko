#pragma once

#include "Callable.h"
#include "Token.h"
#include <string_view>
#include <map>

class Class;

class ClassInstance
{
public:
    ClassInstance(const Class& definition) : m_definition(definition) {}

    const Class& ClassDefinition() const { return m_definition; }

    bool GetProperty(const Token& name, Value& result) const
    {
        auto it = m_properties.find(name.m_lexeme);
        const bool propertyFound = it != m_properties.end();
        if (propertyFound)
        {
            result = it->second;
        } 

        return propertyFound;
    }
private:
    const Class& m_definition;
    std::map<std::string_view, Value> m_properties;
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