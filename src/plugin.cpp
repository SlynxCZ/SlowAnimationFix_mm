// Author: Michal Přikryl (Slynx) <github.com/SlynxCZ>

#include "plugin.h"
#include "scheduler.h"
#include "utils.hpp"

#include "CBasePlayerController.h"

#include "memaddr.hpp"
#include "module.hpp"

#include "eiface.h"
#include "entitysystem.h"
#include "icvar.h"
#include "iserver.h"
#include "interfaces/interfaces.h"
#include "tier1/convar.h"

#include <cstdint>
#include <sstream>
#include <cstdio>
#include <iomanip>
#include <unordered_set>

#define VERSION_STRING SEMVER " @ " GITHUB_SHA
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

using namespace DynLibUtils;

CConVarRef<float> mp_timelimit("mp_timelimit");

static constexpr float CHECK_INTERVAL = 1800.0f; // 30 minutes

static double g_dMapStartUniversalTime = 0.0;
static float g_fPendingTimelimitAdjust = -1.0f;

Plugin g_Plugin;
PLUGIN_EXPOSE(Plugin, g_Plugin);

class GameSessionConfiguration_t
{
};

SH_DECL_HOOK3_void(ISource2Server, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);

int CountConnectedPlayers();
void DoChangelevel();
void OnCheckTimer();

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS();

    GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2Server, ISource2Server, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngineServer, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);

    {
        m_iGameFrameHookID = SH_ADD_HOOK(ISource2Server, GameFrame, g_pSource2Server, SH_MEMBER(this, &Plugin::Hook_GameFrame), false);
        m_iStartupServerHookID = SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &Plugin::Hook_StartupServer), false);
    }

    scheduler::Init();
    scheduler::AddTimer(CHECK_INTERVAL, OnCheckTimer, TIMER_FLAG_REPEAT);

    g_SMAPI->AddListener(this, this);

    return true;
}

bool Plugin::Unload(char* error, size_t maxlen)
{
    scheduler::Shutdown();

    SH_REMOVE_HOOK_ID(m_iGameFrameHookID);
    SH_REMOVE_HOOK_ID(m_iStartupServerHookID);

    return true;
}

void Plugin::Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick)
{
    scheduler::Tick(simulating);
    RETURN_META(MRES_IGNORED);
}

void Plugin::Hook_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession* pWorldSession, const char*)
{
    scheduler::RemoveMapChangeTimers();

    g_dMapStartUniversalTime = g_dUniversalTime;

    if (g_fPendingTimelimitAdjust >= 0.0f)
    {
        float adjusted = g_fPendingTimelimitAdjust;
        g_fPendingTimelimitAdjust = -1.0f;

        scheduler::NextFrame([adjusted]()
        {
            char cmd[64];
            V_snprintf(cmd, sizeof(cmd), "mp_timelimit %.1f\n", (adjusted > 0.0f ? adjusted : 0.1f));
            g_pEngineServer->ServerCommand(cmd);
        });
    }

    RETURN_META(MRES_IGNORED);
}

int CountConnectedPlayers()
{
    CGlobalVars* gpGlobals = g_pEngineServer->GetServerGlobals();
    if (!gpGlobals) return 0;

    int count = 0;
    for (int i = 0; i < gpGlobals->maxClients; i++)
    {
        auto* controller = static_cast<CBasePlayerController*>(GameEntitySystem()->GetEntityInstance(CEntityIndex(i + 1)));
        if (controller && controller->m_iConnected() == PlayerConnectedState::Connected)
            ++count;
    }
    return count;
}

void DoChangelevel()
{
    CNetworkGameServerBase* pGameServer = g_pNetworkServerService->GetIGameServer();
    if (!pGameServer) return;

    const char* mapName = pGameServer->GetMapName();
    if (!mapName || mapName[0] == '\0') return;

    double elapsedSeconds = g_dUniversalTime - g_dMapStartUniversalTime;
    float elapsedMinutes = static_cast<float>(elapsedSeconds) / 60.0f;

    float originalTimelimit = 0.0f;
    if (mp_timelimit.IsValidRef())
        originalTimelimit = mp_timelimit.Get();

    if (originalTimelimit > 0.0f)
        g_fPendingTimelimitAdjust = originalTimelimit - elapsedMinutes;

    if (g_pEngineServer->IsMapValid(mapName))
    {
        g_pEngineServer->ChangeLevel(mapName, nullptr);
    }
    else
    {
        char cmd[512];
        V_snprintf(cmd, sizeof(cmd), "dg_workshop_changelevel %s\n", mapName);
        g_pEngineServer->ServerCommand(cmd);
    }
}

void OnCheckTimer()
{
    if (CountConnectedPlayers() == 0)
        DoChangelevel();
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
