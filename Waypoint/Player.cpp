#include "Player.h"
#include "meta_api.h"

static Player s_Players[32];

Player *GetPlayer(edict_t* pClient)
{
	int index = ENTINDEX(pClient);
	if (index >= 1 && index <= 32)
		return &s_Players[index - 1];

	return nullptr;
}