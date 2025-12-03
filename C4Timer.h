#ifndef _INCLUDE_METAMOD_SOURCE_C4TIMER_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_C4TIMER_PLUGIN_H_

#include <ISmmPlugin.h>
#include <entity2/entitysystem.h>
#include "filesystem.h"
#include "include/menus.h"

class C4TimerPlugin final : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	void AllPluginsLoaded();

private:
	const char* GetAuthor();
	const char* GetName();
	const char* GetDescription();
	const char* GetURL();
	const char* GetLicense();
	const char* GetVersion();
	const char* GetDate();
	const char* GetLogTag();
};

extern CGameEntitySystem* g_pGameEntitySystem;
extern CGlobalVars* gpGlobals;
extern IFileSystem* g_pFullFileSystem;
extern IUtilsApi* g_pUtils;

#endif
