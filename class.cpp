#include "Class.h"
#include "function.h"

ClassInstance::ClassInstance(const Class& definition)
    : m_definition(definition) {}

bool ClassInstance::GetProperty(const Token &name, Value &result) const
{
    auto it = m_properties.find(name.m_lexeme);
    if (it != m_properties.end())
    {
        result = it->second;
        return true;
    }
    else if (const Function *method = m_definition.GetMethod(name.m_lexeme))
    {
        result = Value(method);
        return true;
    }

    return false;
}

Class::Class(std::string_view name, std::map<std::string_view, const Function *> &&methods)
    : m_name(name), m_methods(std::move(methods))
{
}

Value Class::CreateInstance() const
{
    std::shared_ptr<ClassInstance> instance = std::make_shared<ClassInstance>(*this);
    return Value(instance);
}

const Function* Class::GetMethod(std::string_view name) const
{
    auto it = m_methods.find(name);
    if (it != m_methods.end())
    {
        return it->second;
    }

    return nullptr;
}
