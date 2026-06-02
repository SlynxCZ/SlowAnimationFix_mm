//====== Copyright �, Valve Corporation, All rights reserved. =======
//
// Purpose: Additional shared object cache functionality for the GC
//
//=============================================================================

#ifndef SCHEMASYSTEM_HELPER_H
#define SCHEMASYSTEM_HELPER_H
#ifdef _WIN32
#pragma once
#endif

#include <cstdint>
#include <type_traits>

int GetServerPropOffset(const char* pszClassName, const char* pszPropName);

#define SCHEMA_FIELD(type, className, propName)                                                        \
    std::add_lvalue_reference_t<type> propName()                                                       \
    {                                                                                                  \
        static const int32_t offset = GetServerPropOffset(#className, #propName);                      \
        return *reinterpret_cast<std::add_pointer_t<type>>(reinterpret_cast<intptr_t>(this) + offset); \
    }

#define SCHEMA_FIELD_POINTER(type, className, propName)                             \
    type* propName()                                                                \
    {                                                                               \
        static const int32_t offset = GetServerPropOffset(#className, #propName);   \
        return reinterpret_cast<type*>(reinterpret_cast<intptr_t>(this) + offset);  \
    }

#endif //SCHEMASYSTEM_HELPER_H
