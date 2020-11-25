#pragma once

#include "Node.h"

class Player
{
public:
	Player()
	{
		m_IsOnGround = false;
	}

	bool m_IsOnGround;
	Vector m_LastOrigin;
	std::weak_ptr<Node> m_pLastGroundNode;
	std::weak_ptr<Node> m_pLastNode;
};

Player *GetPlayer(edict_t* pClient);