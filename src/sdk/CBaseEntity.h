#pragma once
#include <entityinstance.h>
#include "schemasystem_helper.h"

class CBaseEntity : public CEntityInstance
{
public:
	SCHEMA_FIELD(int32_t, CBaseEntity, m_iHealth);
	SCHEMA_FIELD(int32_t, CBaseEntity, m_iMaxHealth);
	SCHEMA_FIELD(LifeState_t, CBaseEntity, m_lifeState);
	SCHEMA_FIELD(uint8_t, CBaseEntity, m_iTeamNum);
    SCHEMA_FIELD(uint32_t, CBaseEntity, m_fFlags);
};