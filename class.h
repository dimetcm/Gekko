#pragma once

#include "Token.h"
#include <string_view>
#include <map>

class Class;
class Function;

class ClassInstance
{
public:
    explicit ClassInstance(const Class& definition);

    const Class& ClassDefinition() const { return m_definition; }

    bool GetProperty(const Token& name, Value& result) const;
    void SetProperty(const Token& name, Value value)
    {
        m_properties[name.m_lexeme] = value;
    }
private:
    const Class& m_definition;
    std::map<std::string_view, Value> m_properties;
};

class Class
{
public:
    Class(std::string_view name, std::map<std::string_view, const Function*>&& methods);

    Value CreateInstance() const;
    
    std::string_view ToString() const { return m_name; }

    const Function* GetMethod(std::string_view name) const;
private:
    std::string_view m_name;
    std::map<std::string_view, const Function*> m_methods;
};