#include "amxxmodule.h"
#include <usercmd.h>
#include "Editor.h"
#include "NodeManager.h"
#include "amxxapi.h"

void OnAmxxAttach()
{
}

void OnPluginsLoaded()
{
	GetEditor().Init();

	SERVER_PRINT("Waypoint module loaded...\n");
}

void ServerDeactivate()
{
	GetNodeManager().Clear();
	GetEditor().Reset();
}

void ClientDisconnect(edict_t* pEntity)
{
	GetEditor().Disconnect(pEntity);
	RETURN_META(MRES_IGNORED);
}

void ClientCommand(edict_t* pEntity)
{
	GetEditor().HandleCommands(pEntity);
}

void PlayerPreThink(edict_t* pEntity)
{
	GetEditor().Think(pEntity);
	RETURN_META(MRES_IGNORED);
}

void CmdStart(const edict_t* player, const struct usercmd_s* cmd, unsigned int random_seed)
{
	GetEditor().CmdStart(player, cmd);
}