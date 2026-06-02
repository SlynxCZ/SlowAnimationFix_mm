#pragma once
#include "ehandle.h"
#include "CBaseEntity.h"
#include "schemasystem_helper.h"

class CBasePlayerPawn;
class CCSPlayerPawn;

enum class PlayerConnectedState : int32_t
{
    NeverConnected = -1,
    Connected = 0,
    Connecting = 1,
    Reconnecting = 2,
    Disconnecting = 3,
    Disconnected = 4,
    Reserved = 5,
};

class CBasePlayerController : public CBaseEntity
{
public:
    SCHEMA_FIELD(CHandle<CBasePlayerPawn>, CBasePlayerController, m_hPawn);
    SCHEMA_FIELD(uint64_t, CBasePlayerController, m_steamID);
    SCHEMA_FIELD(PlayerConnectedState, CBasePlayerController, m_iConnected);
};

class CCSPlayerController : public CBasePlayerController
{
public:
    SCHEMA_FIELD(CHandle<CCSPlayerPawn>, CCSPlayerController, m_hPlayerPawn);
};
