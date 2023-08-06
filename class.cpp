#include "class.h"
#include "function.h"

ClassInstance::ClassInstance(const Class& definition)
    : m_definition(definition) {}

Class::Class(std::string_view name,
     std::map<std::string_view, const Function *> &&methods,
     std::map<std::string_view, const Function*>&& staticMethods)
    : m_name(name),
    m_methods(std::move(methods)),
    m_staticMethods(std::move(staticMethods))
{
}

std::shared_ptr<ClassInstance> Class::CreateInstance() const
{
    return std::make_shared<ClassInstance>(*this);
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

const Function* Class::GetStaticMethod(std::string_view name) const
{
    auto it = m_staticMethods.find(name);
    if (it != m_staticMethods.end())
    {
        return it->second;
    }

    return nullptr;
}
