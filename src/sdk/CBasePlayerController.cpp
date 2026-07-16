//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose:
//
//=============================================================================

#include "CBasePlayerController.h"
#include "memaddr.hpp"

bool CBasePlayerController::IsBot()
{
    return (m_fFlags() & FL_FAKECLIENT) != 0;
}

bool CBasePlayerController::IsConnected()
{
    return m_iConnected() == PlayerConnectedState::PlayerConnected;
}