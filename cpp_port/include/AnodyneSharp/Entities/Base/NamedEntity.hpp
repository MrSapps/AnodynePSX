#pragma once
#include "AnodyneSharp/Common.hpp"

// Mirrors C# [NamedEntity] attribute
// In C++ we use a macro + registration

#define NAMED_ENTITY(xmlName) \
    static constexpr const char* XmlEntityName = xmlName;

#define NAMED_ENTITY_DEFAULT() \
    static constexpr const char* XmlEntityName = nullptr;

namespace AnodyneSharp::Entities {

struct NamedEntityInfo {
    const char* xmlName = nullptr;
    const char* type    = nullptr;
    const char* map     = nullptr;
    std::vector<int> frames;

    std::string GetName(const std::string& typeName) const {
        return xmlName ? std::string(xmlName) : typeName;
    }
    bool Matches(int f, const std::string& t, const std::string& m) const {
        if (map && m != map) return false;
        if (type && t != type) return false;
        if (!frames.empty()) {
            bool found = false;
            for (int fr : frames) if (fr == f) { found = true; break; }
            if (!found) return false;
        }
        return true;
    }
};

} // namespace AnodyneSharp::Entities
