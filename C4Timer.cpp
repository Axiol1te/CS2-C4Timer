#include "C4Timer.h"
#include "metamod_oslink.h"
#include <ISmmPlugin.h>
#include <KeyValues.h>
#include "igameevents.h"
#include <map>

CGameEntitySystem* GameEntitySystem()
{
	return g_pGameEntitySystem;
}

C4TimerPlugin g_Plugin;
PLUGIN_EXPOSE(C4TimerPlugin, g_Plugin);

CGameEntitySystem* g_pGameEntitySystem = nullptr;
CGlobalVars* gpGlobals = nullptr;
IUtilsApi* g_pUtils = nullptr;

bool g_bBombPlanted = false;
float g_fBombPlantTime = 0.0f;
int g_iBombSite = 0;
int g_iShowTimerFrom = 40;
bool g_bShowSite = true;
bool g_bTimerEnabled = true;

std::map<std::string, std::string> g_Phrases;
bool g_bTranslationsLoaded = false;

void LoadTranslationsLazy()
{
    if (g_bTranslationsLoaded || !g_pFullFileSystem || !g_pUtils) return;
    
    KeyValues* kvPhrases = new KeyValues("Phrases");
    const char* pszPath = "addons/translations/C4Timer.phrases.txt";
    
    if (kvPhrases->LoadFromFile(g_pFullFileSystem, pszPath))
    {
        const char* pszLanguage = g_pUtils->GetLanguage();
        
        for (KeyValues *pKey = kvPhrases->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey())
        {
            const char* keyName = pKey->GetName();
            const char* translation = pKey->GetString(pszLanguage);
            
            if (translation && translation[0] != '\0')
            {
                g_Phrases[std::string(keyName)] = std::string(translation);
            }
        }
        
        g_bTranslationsLoaded = true;
    }
    
    delete kvPhrases;
}

std::string FormatTranslation(const char* key, const char* param1, const char* param2)
{
    LoadTranslationsLazy();
    
    auto it = g_Phrases.find(key);
    std::string result = (it != g_Phrases.end()) ? it->second : key;

    if (param1 && param1[0] != '\0')
    {
        size_t pos = result.find("{1}");
        if (pos != std::string::npos) result.replace(pos, 3, param1);
    }
    
    if (param2 && param2[0] != '\0')
    {
        size_t pos = result.find("{2}");
        if (pos != std::string::npos) result.replace(pos, 3, param2);
    }
    
    return result;
}

void LoadConfig()
{
    if (!g_pFullFileSystem) return;
    
    KeyValues* kvConfig = new KeyValues("Settings");
    const char* pszPath = "addons/configs/C4Timer/core.ini";
    
    if (kvConfig->LoadFromFile(g_pFullFileSystem, pszPath))
    {
        g_iShowTimerFrom = kvConfig->GetInt("ShowTimerFrom", 40);
        g_bShowSite = kvConfig->GetInt("ShowSite", 1) != 0;
        g_bTimerEnabled = kvConfig->GetInt("TimerEnabled", 1) != 0;
        
        if (g_iShowTimerFrom < 1) g_iShowTimerFrom = 1;
        if (g_iShowTimerFrom > 40) g_iShowTimerFrom = 40;
    }
    
    delete kvConfig;
}

void UpdateC4Timer()
{
    if (!g_bTimerEnabled || !g_bBombPlanted || !gpGlobals || !g_pUtils) return;

    float currentTime = gpGlobals->curtime;
    float timeSincePlant = currentTime - g_fBombPlantTime;
    int timeleft = 40 - (int)timeSincePlant;

    if (timeleft <= 0)
    {
        g_bBombPlanted = false;
        return;
    }
    
    if (timeleft > g_iShowTimerFrom) return;

    const char* sitename = (g_iBombSite == 1) ? "A" : "B";
    char message[512];
    
    int totalBars = 30;
    int filledBars = (timeleft * totalBars) / 40;
    
    char progressBar[128];
    int pos = 0;
    
    progressBar[pos++] = '[';
    for (int i = 0; i < totalBars; i++)
    {
        progressBar[pos++] = (i < filledBars) ? '=' : '-';
    }
    progressBar[pos++] = ']';
    progressBar[pos] = '\0';

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d", timeleft);
    
    if (g_bShowSite)
    {
        std::string text = FormatTranslation("C4Timer_OnSite", sitename, timeStr);
        snprintf(message, sizeof(message), "%s\n%s", text.c_str(), progressBar);
    }
    else
    {
        std::string text = FormatTranslation("C4Timer_NoSite", timeStr, nullptr);
        snprintf(message, sizeof(message), "%s\n%s", text.c_str(), progressBar);
    }

    g_pUtils->PrintToCenterAll(message);
}

void InitC4Timer()
{
    if (!g_pUtils || !gpGlobals) return;
    
    g_pUtils->CreateTimer(1.0f, []() {
        UpdateC4Timer();
        return 1.0f;
    });

    g_pUtils->HookEvent(g_PLID, "bomb_planted",
        [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
    {
        if (!g_bTimerEnabled) return;
        
        int site = pEvent->GetInt("site");
        int siteIndex = site & 0x01;
        
        g_bBombPlanted = true;
        g_iBombSite = siteIndex;
        g_fBombPlantTime = gpGlobals->curtime;
    });
    
    g_pUtils->HookEvent(g_PLID, "bomb_exploded",
        [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
    {
        if (!g_bTimerEnabled) return;
        g_bBombPlanted = false;
    });
    
    g_pUtils->HookEvent(g_PLID, "bomb_defused",
        [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
    {
        if (!g_bTimerEnabled) return;
        g_bBombPlanted = false;
    });
    
    g_pUtils->HookEvent(g_PLID, "round_end",
        [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
    {
        if (!g_bTimerEnabled) return;
        g_bBombPlanted = false;
    });
    
    g_pUtils->HookEvent(g_PLID, "round_start",
        [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
    {
        if (!g_bTimerEnabled) return;
        g_bBombPlanted = false;
    });
}

bool C4TimerPlugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	return true;
}

void C4TimerPlugin::AllPluginsLoaded()
{
	int ret;
	g_pUtils = (IUtilsApi*)g_SMAPI->MetaFactory(Utils_INTERFACE, &ret, nullptr);
	if (ret == META_IFACE_FAILED || !g_pUtils) return;

	g_pUtils->StartupServer(g_PLID, []() {
		g_pGameEntitySystem = g_pUtils->GetCGameEntitySystem();
		gpGlobals = g_pUtils->GetCGlobalVars();
		LoadConfig();
		InitC4Timer();
	});

	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);
}

bool C4TimerPlugin::Unload(char* error, size_t maxlen)
{
	ConVar_Unregister();
	return true;
}

const char* C4TimerPlugin::GetAuthor()
{
	return "Axiolite";
}

const char* C4TimerPlugin::GetName()
{
	return "C4Timer";
}

const char* C4TimerPlugin::GetDescription()
{
	return "Таймер бомбы";
}

const char* C4TimerPlugin::GetURL()
{
	return "https://github.com/Axiol1te";
}

const char* C4TimerPlugin::GetLicense()
{
	return "GPL";
}

const char* C4TimerPlugin::GetVersion()
{
	return "1.0.0";
}

const char* C4TimerPlugin::GetDate()
{
	return __DATE__;
}

const char* C4TimerPlugin::GetLogTag()
{
	return "C4TIMER";
}
