#include "amxxmodule.h"
#include <usercmd.h>
#include "Waypoint.h"
#include "Command.h"
#include "amxxapi.h"

NodeManager g_NodeManager;
PathFinder g_PathFinder;

short g_SpriteBeam1;
short g_SpriteBeam4;
short g_SpriteArrow;

char g_pszFilePath[256];

void OnAmxxAttach()
{
	WP_RegisterFunctions();
}

void OnPluginsLoaded()
{
	g_SpriteBeam1 = PRECACHE_MODEL("sprites/zbeam1.spr");
	g_SpriteBeam4 = PRECACHE_MODEL("sprites/zbeam4.spr");
	g_SpriteArrow = PRECACHE_MODEL("sprites/arrow1.spr");

	MF_BuildPathnameR(g_pszFilePath, sizeof(g_pszFilePath), "%s/waypoints/%s.txt", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"), STRING(gpGlobals->mapname));
	g_NodeManager.LoadFile(g_pszFilePath);

	EditorInit();

	SERVER_PRINT("Waypoint module loaded...\n");
}

void ServerDeactivate()
{
	g_NodeManager.Clear();
	EditorReset();
}

void ClientDisconnect(edict_t* pEntity)
{
	EditorReset();
	SERVER_PRINT("Client disconnected...\n");
	RETURN_META(MRES_IGNORED);
}

void ClientCommand(edict_t* pEntity)
{
	EditorCommands(pEntity);
}

void PlayerPreThink(edict_t* pEntity)
{
	EditorThink(pEntity);
}

void CmdStart(const edict_t* player, const struct usercmd_s* cmd, unsigned int random_seed)
{
	EditorCmdStart(player, cmd);
}