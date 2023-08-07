#include "class.h"
#include "function.h"

ClassInstance::ClassInstance(const Class& definition)
    : m_definition(definition) {}

Class::Class(std::string_view name,
     std::map<std::string_view, const Function *> &&methods,
     std::map<std::string_view, const Function*>&& staticMethods,
     std::map<std::string_view, const Function*>&& getters)
    : m_name(name),
    m_methods(std::move(methods)),
    m_staticMethods(std::move(staticMethods)),
    m_getters(std::move(getters))
{
}

std::shared_ptr<ClassInstance> Class::CreateInstance() const
{
    return std::make_shared<ClassInstance>(*this);
}

static const Function* GetMethodByName(const std::map<std::string_view, const Function*>& methods, std::string_view name)
{
    auto it = methods.find(name);
    if (it != methods.end())
    {
        return it->second;
    }

    return nullptr;

}

const Function* Class::GetMethod(std::string_view name) const
{
    return GetMethodByName(m_methods, name);
}

const Function* Class::GetStaticMethod(std::string_view name) const
{
    return GetMethodByName(m_staticMethods, name);
}

const Function* Class::GetGetter(std::string_view name) const
{
    return GetMethodByName(m_getters, name);
}