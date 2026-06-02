// Author: Michal Přikryl (Slynx) <github.com/SlynxCZ>

#include "plugin.h"
#include "utils.hpp"

#include "CBasePlayerController.h"

#include "memaddr.hpp"
#include "module.hpp"

#include "eiface.h"
#include "iserver.h"
#include "interfaces/interfaces.h"
#include "entitysystem.h"

#include <cstdint>
#include <sstream>
#include <cstdio>
#include <iomanip>
#include <unordered_set>

#define VERSION_STRING SEMVER " @ " GITHUB_SHA
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

using namespace DynLibUtils;

Plugin g_Plugin;
PLUGIN_EXPOSE(Plugin, g_Plugin);

SH_DECL_HOOK3_void(ISource2Server, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);

    // VirtualTable Hooks
    {
        m_iGameFrameHookID = SH_ADD_HOOK(ISource2Server, GameFrame, g_pSource2Server, SH_MEMBER(this, &Plugin::CSource2Server_GameFrame), false);
    }

    g_SMAPI->AddListener(this, this);

    return true;
}

bool Plugin::Unload(char* error, size_t maxlen)
{
    SH_REMOVE_HOOK_ID(m_iGameFrameHookID);

    return true;
}

void Plugin::CSource2Server_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
    if (!simulating)
        RETURN_META(MRES_IGNORED);

    CGlobalVars* gpGlobals = g_pEngineServer->GetServerGlobals();
    if (!gpGlobals)
        RETURN_META(MRES_IGNORED);

    constexpr float CURTIME_THRESHOLD = 3600.0f; // reset after 1h idle
    constexpr float CURTIME_BASELINE  = 60.0f; // lets use 60s as baseline

    float& curtime = gpGlobals->curtime;
    int& tickcount = gpGlobals->tickcount;
    float ipt = *CMemory(gpGlobals).Offset(0x54).RCast<float*>();

    if (curtime < CURTIME_THRESHOLD)
        RETURN_META(MRES_IGNORED);

    for (int i = 0; i < gpGlobals->maxClients; i++)
    {
        auto controller = static_cast<CBasePlayerController*>(GameEntitySystem()->GetEntityInstance(CEntityIndex(i + 1)));
        if(controller)
        {
            if (controller->m_iConnected() == PlayerConnectedState::Connected)
            {
                RETURN_META(MRES_IGNORED);
            }
        }
    }

    float offset = curtime - CURTIME_BASELINE;
    curtime -= offset;
    tickcount = static_cast<int>(CURTIME_BASELINE / ipt);

    RETURN_META(MRES_IGNORED);
}

///////////////////////////////////////

CGameEntitySystem* GameEntitySystem()
{
    // CGameResourceService::SetEntityResourceManifest
    // str server_entities
    return *CMemory(g_pGameResourceServiceServer).Offset(WIN_LINUX(0x58, 0x50)).RCast<CGameEntitySystem**>();
}

///////////////////////////////////////
const char* Plugin::GetLicense()
{
    return "GPLv3";
}

const char* Plugin::GetVersion()
{
    return VERSION_STRING;
}

const char* Plugin::GetDate()
{
    return BUILD_TIMESTAMP;
}

const char* Plugin::GetLogTag()
{
    return "SlowAnimationFix";
}

const char* Plugin::GetAuthor()
{
    return "Slynx (˙·٠● S l y n x ●٠·˙)";
}

const char* Plugin::GetDescription()
{
    return "Slow animation fix";
}

const char* Plugin::GetName()
{
    return "Slow animation fix";
}

const char* Plugin::GetURL()
{
    return "https://slynxdev.cz";
}
