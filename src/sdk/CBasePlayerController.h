#pragma once
#include "ehandle.h"
#include "CBaseEntity.h"
#include "schemasystem_helper.h"

enum class PlayerConnectedState : uint32_t
{
    PlayerNeverConnected = 0xFFFFFFFF,
    PlayerConnected = 0x0,
    PlayerConnecting = 0x1,
    PlayerReconnecting = 0x2,
    PlayerDisconnecting = 0x3,
    PlayerDisconnected = 0x4,
    PlayerReserved = 0x5,
};

class CBasePlayerController : public CBaseEntity
{
public:
	SCHEMA_FIELD(PlayerConnectedState, CBasePlayerController, m_iConnected);
	SCHEMA_FIELD(bool, CBasePlayerController, m_bIsHLTV);

    bool IsBot();
    bool IsConnected();
    bool IsHLTV();
};