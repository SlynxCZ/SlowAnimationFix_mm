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

#include <cstdio>
#include <tier0/dbg.h>

#define VERSION_STRING SEMVER " @ " GITHUB_SHA
#define BUILD_TIMESTAMP __DATE__ " " __TIME__

using namespace DynLibUtils;

Plugin g_Plugin;
PLUGIN_EXPOSE(Plugin, g_Plugin);

// Snapshot of the current map name, taken at StartupServer (map start)
char g_szMap[256] = "";

// Reference plugin reloads the map every 30 minutes when the server is empty
constexpr float MAP_RELOAD_INTERVAL = 1800.0f;

void OnMapReloadTimer();

class GameSessionConfiguration_t
{
};

SH_DECL_HOOK3_void(ISource2Server, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);

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
        m_iGameFrameHookID = SH_ADD_HOOK(ISource2Server, GameFrame, g_pSource2Server, SH_MEMBER(this, &Plugin::Hook_GameFrame), true);
        m_iStartupServerHookID = SH_ADD_HOOK(INetworkServerService, StartupServer, g_pNetworkServerService, SH_MEMBER(this, &Plugin::Hook_StartupServer), true);
    }

    scheduler::Init();

    g_SMAPI->AddListener(this, this);

    return true;
}

void Plugin::AllPluginsLoaded()
{
    scheduler::AddTimer(MAP_RELOAD_INTERVAL, OnMapReloadTimer, TIMER_FLAG_REPEAT);
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

    const char* mapName = nullptr;
    if (CNetworkGameServerBase* pGameServer = g_pNetworkServerService->GetIGameServer())
        mapName = pGameServer->GetMapName();

    V_snprintf(g_szMap, sizeof(g_szMap), "%s", (mapName && mapName[0]) ? mapName : "unknown");

    META_LOG(this, "StartupServer: map snapshot = '%s'\n", g_szMap);

    RETURN_META(MRES_IGNORED);
}

void OnMapReloadTimer()
{
    CGameEntitySystem* pEntitySystem = GameEntitySystem();
    if (!pEntitySystem)
        return;

    int iPlayers = 0;
    for (int i = 0; i < 64; i++)
    {
        auto* pController = static_cast<CBasePlayerController*>(pEntitySystem->GetEntityInstance(CEntityIndex(i + 1)));
        if (!pController || pController->IsBot() || pController->IsHLTV() || !pController->IsConnected())
            continue;

        iPlayers++;
    }

    META_LOG(&g_Plugin, "reload check: map='%s', human players=%d\n", g_szMap, iPlayers);

    if (!g_szMap[0])
    {
        META_LOG(&g_Plugin, "reload skipped: no map snapshot (StartupServer hook did not run?)\n");
        return;
    }

    if (iPlayers > 0)
    {
        META_LOG(&g_Plugin, "reload skipped: %d human player(s) connected, next check in %.0f s\n", iPlayers, MAP_RELOAD_INTERVAL);
        return;
    }

    if (g_pEngineServer->IsMapValid(g_szMap))
    {
        META_LOG(&g_Plugin, "server empty -> ChangeLevel('%s')\n", g_szMap);
        g_pEngineServer->ChangeLevel(g_szMap, nullptr);
    }
    else
    {
        char szBuffer[256];
        V_snprintf(szBuffer, sizeof(szBuffer), "ds_workshop_changelevel %s", g_szMap);
        META_LOG(&g_Plugin, "server empty, map not valid as regular map -> '%s'\n", szBuffer);
        g_pEngineServer->ServerCommand(szBuffer);
    }
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
    return "Slynx (˙·٠● S l y n x ●٠·˙)";
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
