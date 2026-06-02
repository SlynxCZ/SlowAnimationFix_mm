#include <schemasystem/schemasystem.h>
#include "schemasystem_helper.h"
#include "utils.hpp"
#include <cstring>

int GetServerPropOffset(const char* pszClassName, const char* pszPropName)
{
    static CSchemaSystemTypeScope* pType = g_pSchemaSystem->FindTypeScopeForModule(WIN_LINUX("server.dll", "libserver.so"));

    SchemaClassInfoData_t* pClassInfo = pType->FindDeclaredClass(pszClassName).Get();
    if (!pClassInfo)
    {
        Error("GetServerOffset: '%s' was not found!\n", pszClassName);
        return -1;
    }

    for (uint16 i = 0; i < pClassInfo->m_nFieldCount; ++i)
    {
        SchemaClassFieldData_t& fieldData = pClassInfo->m_pFields[i];

        if (std::strcmp(fieldData.m_pszName, pszPropName) == 0)
        {
            return fieldData.m_nSingleInheritanceOffset;
        }
    }

    Error("GetServerOffset: '%s::%s' was not found!\n", pszClassName, pszPropName);
    return -1;
}
