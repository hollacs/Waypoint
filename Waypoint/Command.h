#pragma once

#include "extdll.h"
#include <usercmd.h>

void EditorInit();
void EditorCommands(edict_t* pClient);
void EditorThink(edict_t* pEditor);
void EditorCmdStart(const edict_t* player, const struct usercmd_s* cmd);
void EditorReset();