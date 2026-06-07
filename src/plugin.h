#ifndef _INCLUDE_SLOW_ANIMATION_FIX_PLUGIN_SLYNX_H_
#define _INCLUDE_SLOW_ANIMATION_FIX_PLUGIN_SLYNX_H_
#ifdef _WIN32
#pragma once
#endif

#include "inetchannel.h"
#include "ISmmPlugin.h"
#include "iserver.h"

class Plugin final : public ISmmPlugin, IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late) override;
	bool Unload(char* error, size_t maxlen) override;

private:
	const char* GetAuthor() override;
	const char* GetName() override;
	const char* GetDescription() override;
	const char* GetURL() override;
	const char* GetLicense() override;
	const char* GetVersion() override;
	const char* GetDate() override;
	const char* GetLogTag() override;

public:
	void Hook_GameFrame(bool simulating, bool bFirstTick, bool bLastTick);
	void Hook_StartupServer(const GameSessionConfiguration_t& config, ISource2WorldSession* pWorldSession, const char*);

	int m_iGameFrameHookID;
	int m_iStartupServerHookID;
};

extern Plugin g_Plugin;

PLUGIN_GLOBALVARS();

#endif // _INCLUDE_SLOW_ANIMATION_FIX_PLUGIN_SLYNX_H_
