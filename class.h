#pragma once

#include "Token.h"
#include <string_view>
#include <map>

class Class;
class Function;

struct ClassInstance
{
    explicit ClassInstance(const Class& definition);

    const Class& ClassDefinition() const { return m_definition; }

    const Class& m_definition;
    std::map<std::string_view, Value> m_properties;
};

class Class
{
public:
    Class(std::string_view name,
        std::map<std::string_view, const Function*>&& methods,
        std::map<std::string_view, const Function*>&& staticMethods,
        std::map<std::string_view, const Function*>&& getters);

    std::shared_ptr<ClassInstance> CreateInstance() const;

    std::string_view ToString() const { return m_name; }

    const Function* GetMethod(std::string_view name) const;
    const Function* GetStaticMethod(std::string_view name) const;
    const Function* GetGetter(std::string_view name) const;
private:
    std::string_view m_name;
    std::map<std::string_view, const Function*> m_methods;
    std::map<std::string_view, const Function*> m_staticMethods;
    std::map<std::string_view, const Function*> m_getters;
};