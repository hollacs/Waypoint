#include "Editor.h"

#include <sstream>

#include "amxxmodule.h"
#include "usercmd.h"

#include "NodeManager.h"
#include "PathFinder.h"
#include "Utils.h"

#define AUTO_NODE_DIST 100.0f
#define AUTO_PATH_DIST 180.0f
#define AUTO_MAX_DIST 220.0f
#define AUTO_MAX_JUMP_DIST 220.0f
#define AUTO_MAX_HEIGHT 175.0f
#define AUTO_MIN_HEIGHT 32.0f

static Editor s_Editor;
static short s_spr_zbeam1, s_spr_zbeam4, s_spr_arrow1;

Editor::Editor()
{
	m_pEditor = nullptr;
	m_EditTime = 0.0f;
	m_AutoTime = 0.0f;
	m_AutoMode = false;
	m_AutoNodeDist = AUTO_NODE_DIST;
	m_AutoPathDist = AUTO_PATH_DIST;
	m_IsOnGround = false;
	m_IsStillOnGround = false;
	strcpy(m_szFilePath, "");
}

void Editor::Init()
{
	this->Reset();

	MF_BuildPathnameR(m_szFilePath, sizeof(m_szFilePath), "%s/waypoints/%s.txt", MF_GetLocalInfo("amxx_configsdir", "addons/amxmodx/configs"), STRING(gpGlobals->mapname));

	GetNodeManager().LoadFile(m_szFilePath);

	s_spr_zbeam1 = PRECACHE_MODEL("sprites/zbeam1.spr");
	s_spr_zbeam4 = PRECACHE_MODEL("sprites/zbeam4.spr");
	s_spr_arrow1 = PRECACHE_MODEL("sprites/arrow1.spr");
}

void Editor::Reset()
{
	m_pEditor = nullptr;
	m_EditTime = 0.0f;
	m_AutoTime = 0.0f;
	m_AutoMode = false;
	m_AutoNodeDist = AUTO_NODE_DIST;
	m_AutoPathDist = AUTO_PATH_DIST;
	m_IsOnGround = false;
	m_IsStillOnGround = false;
}

void Editor::HandleCommands(edict_t* pClient)
{
	const char* pszCmd = CMD_ARGV(0);

	if (!FStrEq(pszCmd, "wp"))
	{
		RETURN_META(MRES_IGNORED);
	}

	pszCmd = CMD_ARGV(1);

	const char* pszArg1, * pszArg2, * pszArg3;

	pszArg1 = CMD_ARGV(2);
	pszArg2 = CMD_ARGV(3);
	pszArg3 = CMD_ARGV(4);

	if (FStrEq(pszCmd, "edit") && CMD_ARGC() == 3)
	{
		if (FStrEq(pszArg1, "1"))
		{
			ToggleEditMode(pClient, true);
		}
		else if (FStrEq(pszArg1, "0"))
		{
			ToggleEditMode(pClient, false);
		}

		RETURN_META(MRES_SUPERCEDE);
	}

	if (m_pEditor != pClient)
	{
		CLIENT_PRINTF(pClient, print_console, "[WP] You are not in the edit mode.\n");
		RETURN_META(MRES_SUPERCEDE);
	}

	if (FStrEq(pszCmd, "create"))
	{
		if (FStrEq(pszArg1, "node"))
		{
			CreateNode(pClient->v.origin, pszArg2);
		}
		else if (FStrEq(pszArg1, "path"))
		{
			CreatePath(pszArg2);
		}
	}
	else if (FStrEq(pszCmd, "auto"))
	{
		if (CMD_ARGC() == 3)
		{
			if (FStrEq(pszArg1, "1"))
			{
				ToggleAutoMode(true);
			}
			else if (FStrEq(pszArg1, "0"))
			{
				ToggleAutoMode(false);
			}
		}
		else if (CMD_ARGC() == 5)
		{
			if (FStrEq(pszArg1, "node") && FStrEq(pszArg2, "dist"))
			{
				SetAutoNode(atof(pszArg3));
			}
			else if (FStrEq(pszArg1, "path") && FStrEq(pszArg2, "dist"))
			{
				SetAutoPath(atof(pszArg3));
			}
		}
	}
	else if (FStrEq(pszCmd, "remove"))
	{
		if (FStrEq(pszArg1, "node"))
		{
			RemoveNode();
		}
		else if (FStrEq(pszArg1, "path"))
		{
			RemovePath();
		}
		else if (FStrEq(pszArg1, "all"))
		{
			RemoveAllNodes();
		}
	}
	else if (FStrEq(pszCmd, "flag"))
	{
		if (FStrEq(pszArg1, "node"))
		{
			if (FStrEq(pszArg2, "+"))
			{
				AddNodeFlag(pszArg3);
			}
			else if (FStrEq(pszArg2, "-"))
			{
				RemoveNodeFlag(pszArg3);
			}
			else if (FStrEq(pszArg2, "="))
			{
				SetNodeFlag(pszArg3);
			}
		}
		else if (FStrEq(pszArg1, "path"))
		{
			if (FStrEq(pszArg2, "+"))
			{
				AddPathFlag(pszArg3);
			}
			else if (FStrEq(pszArg2, "-"))
			{
				RemovePathFlag(pszArg3);
			}
			else if (FStrEq(pszArg2, "="))
			{
				SetPathFlag(pszArg3);
			}
		}
	}
	else if (FStrEq(pszCmd, "save"))
	{
		SaveFile();
	}
	else if (FStrEq(pszCmd, "astar"))
	{
		if (FStrEq(pszArg1, "begin"))
		{
			std::shared_ptr<Node> pNode = GetCurrentNode();

			if (CheckNode(pNode))
			{
				m_pBegin = pNode;
				CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] A*: Set begin node #%d\n", NODE_INDEX(pNode)));
			}
		}
		else if (FStrEq(pszArg1, "end"))
		{
			std::shared_ptr<Node> pNode = GetCurrentNode();

			if (CheckNode(pNode))
			{
				m_pEnd = pNode;
				CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] A*: Set end node #%d\n", NODE_INDEX(pNode)));
			}
		}
		else if (FStrEq(pszArg1, "run"))
		{
			std::shared_ptr<Node> pBegin, pEnd;
			pBegin = m_pBegin.lock();
			pEnd = m_pEnd.lock();

			if (pBegin == nullptr || pEnd == nullptr || pBegin == pEnd)
			{
				CLIENT_PRINTF(pClient, print_console, "[WP] A*: Invalid begin or end node.\n");
			}
			else
			{
				PathResult result;
				if (GetPathFinder().CalcPath(pBegin, pEnd, result))
				{
					for (auto it = result.begin(), previt = it++; it != result.end(); ++it, ++previt)
					{
						UTIL_BeamPoints(pClient, (*previt).lock()->GetPosition(), (*it).lock()->GetPosition(), s_spr_zbeam1, 0, 0, 100, 20, 3, 0, 100, 255, 255, 10);
					}

					CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] A*: Path found (%d)\n", result.size()));
				}
				else
				{
					CLIENT_PRINTF(pClient, print_console, "[WP] A*: Path not found.\n");
				}
			}
		}
	}

	RETURN_META(MRES_SUPERCEDE);
}

void Editor::CmdStart(const edict_t* pPlayer, const usercmd_s* cmd)
{
	if (m_AutoMode && pPlayer == m_pEditor && pPlayer->v.deadflag == DEAD_NO)
	{
		if ((cmd->buttons & IN_JUMP) && (~pPlayer->v.oldbuttons & IN_JUMP) && (pPlayer->v.flags & FL_ONGROUND))
		{
			AutoCreateNode(const_cast<edict_t*>(pPlayer), pPlayer->v.origin, 40.0f, 0, false, false);
			m_IsJumping = true;
		}
	}
}

void Editor::Think(edict_t* pEntity)
{
	if (pEntity != m_pEditor)
		return;

	if (m_AutoMode && pEntity->v.deadflag == DEAD_NO)
	{
		if (pEntity->v.flags & FL_ONGROUND)
		{
			if (!m_IsOnGround)
			{
				AutoCreateNode(pEntity, pEntity->v.origin, 40.0f, 0, false, true);
				m_IsJumping = false;
			}

			m_LastOrigin = pEntity->v.origin;
			m_IsOnGround = true;
		}
		else
		{
			if (m_IsOnGround && !m_IsJumping)
			{
				AutoCreateNode(pEntity, m_LastOrigin, 40.0f, 0, false, false);
			}

			m_IsStillOnGround = false;
			m_IsOnGround = false;
		}

		if (gpGlobals->time >= m_AutoTime + 0.1)
		{
			if (pEntity->v.flags & FL_ONGROUND)
			{
				AutoCreateNode(pEntity, pEntity->v.origin, 36.0f, 0, true, false);
				m_IsJumping = false; // some dealy
			}

			m_AutoTime = gpGlobals->time;
		}
	}

	if (gpGlobals->time >= m_EditTime + 0.5)
	{
		auto nodes = GetNodeManager().GetNodes();

		if (!nodes.empty())
		{
			size_t max_nodes = min(nodes.size(), 40);

			std::shared_ptr<Node> pCurrentNode = nullptr;

			for (size_t i = 0; i < max_nodes; i++)
			{
				std::shared_ptr<Node> pNode = nullptr;
				float min_distance = 1250;
				std::vector<std::shared_ptr<Node>>::iterator node_iter;

				for (auto iter = nodes.begin(); iter != nodes.end(); iter++)
				{
					float distance = ((*iter)->GetPosition() - pEntity->v.origin).Length();
					if (distance < min_distance)
					{
						node_iter = iter;
						pNode = *iter;
						min_distance = distance;
					}
				}

				if (pNode == nullptr)
					break;

				nodes.erase(node_iter);

				if (i == 0 && min_distance <= 50)
				{
					pCurrentNode = pNode;

					const auto& paths = pNode->GetPaths();

					for (auto iter = paths.begin(); iter != paths.end(); iter++)
					{
						std::shared_ptr<Node> pBeginNode = pNode;
						std::shared_ptr<Node> pEndNode = (*iter)->GetEnd();

						DrawPath(pBeginNode, pEndNode);
						DrawNode(pEndNode, false, false);

						nodes.erase(std::remove(nodes.begin(), nodes.end(), pEndNode), nodes.end());
					}

					const auto& nodes_ = GetNodeManager().GetNodes();

					for (auto iter = nodes_.begin(); iter != nodes_.end(); iter++)
					{
						std::shared_ptr<Node> pBeginNode = pNode;
						std::shared_ptr<Node> pEndNode = *iter;

						if (pEndNode->GetPath(pBeginNode) != nullptr && pBeginNode->GetPath(pEndNode) == nullptr)
						{
							DrawPath(pBeginNode, pEndNode);
							DrawNode(pEndNode, false, false);

							nodes.erase(std::remove(nodes.begin(), nodes.end(), pEndNode), nodes.end());
						}
					}

					DrawNode(pNode, true, false);
				}
				else
				{
					DrawNode(pNode, false, false);
				}
			}

			std::string message;

			if (pCurrentNode != nullptr)
			{
				Vector pos = pCurrentNode->GetPosition();

				message += UTIL_VarArgs("[Current Node: #%d]\norigin: {%.f, %.f, %.f}\nflags: {%s}\nconnections: %d\n\n", 
					NODE_INDEX(pCurrentNode), pos.x, pos.y, pos.z, GetNameByFlags(pCurrentNode->GetFlags()).c_str(), pCurrentNode->GetPaths().size());
			}

			std::shared_ptr<Node> pAimNode = GetNodeManager().GetAimNode(pEntity, 16, 1000, pCurrentNode);
			if (pAimNode != nullptr)
			{
				DrawNode(pAimNode, false, true);

				Vector pos = pAimNode->GetPosition();

				message += UTIL_VarArgs("[Aim Node: #%d]\norigin: {%.f, %.f, %.f}\nflags: {%s}\nconnections: %d\n\n",
					NODE_INDEX(pAimNode), pos.x, pos.y, pos.z, GetNameByFlags(pAimNode->GetFlags()).c_str(), pAimNode->GetPaths().size());

				if (pCurrentNode != nullptr)
				{
					std::shared_ptr<Path> pPath = pCurrentNode->GetPath(pAimNode);
					if (pPath != nullptr)
					{
						message += UTIL_VarArgs("\n[Current Path: #%d (#%d --> #%d)]\nflags: {%s}\n",
							PATH_INDEX(pPath), NODE_INDEX(pCurrentNode), NODE_INDEX(pAimNode), GetNameByFlags(pPath->GetFlags()).c_str());
					}
				}
			}

			hudtextparms_t hud;
			hud.a1 = 0;
			hud.a2 = 0;
			hud.r2 = 255;
			hud.g2 = 255;
			hud.b2 = 250;
			hud.r1 = 0;
			hud.g1 = 255;
			hud.b1 = 255;
			hud.x = 0.1;
			hud.y = 0.1;
			hud.effect = 0;
			hud.fxTime = 0.0;
			hud.holdTime = 0.7;
			hud.fadeinTime = 0.0;
			hud.fadeoutTime = 0.5;
			hud.channel = 4;

			UTIL_HudMessage(m_pEditor, hud, message.c_str());
		}

		m_EditTime = gpGlobals->time;
	}
}

void Editor::Disconnect(edict_t* pEntity)
{
	if (pEntity == m_pEditor)
	{
		m_pEditor = nullptr;
	}
}

void Editor::ToggleEditMode(edict_t *pEntity, bool enabled)
{
	if (enabled)
	{
		m_pEditor = pEntity;
		CLIENT_PRINTF(pEntity, print_console, "[WP] Edit mode has been Enabled.\n");
	}
	else
	{
		m_pEditor = nullptr;
		CLIENT_PRINTF(pEntity, print_console, "[WP] Edit mode has been Disabled.\n");
	}
}

void Editor::ToggleAutoMode(bool enabled)
{
	if (enabled)
	{
		m_AutoMode = true;
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] Auto mode has been Enabled.\n");
	}
	else
	{
		m_AutoMode = false;
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] Auto mode has been Disabled.\n");
	}
}

void Editor::CreateNode(const Vector& origin, const char *pszFlags)
{
	int flags = GetFlagsByName(pszFlags);
	GetNodeManager().CreateNode(m_pEditor->v.origin, flags);
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Create node #%d\n", GetNodeManager().GetNodeSize()));
}

void Editor::CreatePath(const char* pszFlags)
{
	std::shared_ptr<Node> pBegin = GetCurrentNode();
	std::shared_ptr<Node> pEnd = GetAimNode(pBegin);

	if (!CheckNodes(pBegin, pEnd, false))
		return;

	int flags = GetFlagsByName(pszFlags);
	std::shared_ptr<Path> pPath = GetNodeManager().CreatePath(pBegin, pEnd, flags);

	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Create path #%d to connect nodes (#%d --> #%d)\n",
		PATH_INDEX(pPath), NODE_INDEX(pBegin), NODE_INDEX(pEnd)));
}

void Editor::RemoveNode()
{
	std::shared_ptr<Node> pNode = GetCurrentNode();
	if (!CheckNode(pNode))
		return;

	size_t index = NODE_INDEX(pNode);
	GetNodeManager().RemoveNode(pNode);
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Remove node #%d\n", index));
}

void Editor::RemoveAllNodes()
{
	GetNodeManager().Clear();
	CLIENT_PRINTF(m_pEditor, print_console, "[WP] Remove all nodes and paths in this map.\n");
}

void Editor::RemovePath()
{
	std::shared_ptr<Node> pBegin = GetCurrentNode();
	std::shared_ptr<Node> pEnd = GetAimNode(pBegin);

	if (!CheckNodes(pBegin, pEnd, true))
		return;

	std::shared_ptr<Path> pPath = pBegin->GetPath(pEnd);
	size_t index = PATH_INDEX(pPath);

	GetNodeManager().RemovePath(pPath);

	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Remove path #%d to disconnect nodes (#%d --x--> #%d)\n",
		index, NODE_INDEX(pBegin), NODE_INDEX(pEnd)));
}

bool Editor::CheckFlags(int flags)
{
	if (flags == 0)
	{
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] Invalid flag name.\n");
		return false;
	}

	return true;
}

void Editor::AddNodeFlag(const char* pszFlag)
{
	std::shared_ptr<Node> pNode = GetCurrentNode();
	if (!CheckNode(pNode))
		return;

	int flag = GetFlagsByName(pszFlag);
	if (!CheckFlags(flag))
		return;

	pNode->SetFlags(pNode->GetFlags() | flag);
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Add flags '%s' for node #%d\n", pszFlag, NODE_INDEX(pNode)));
}

void Editor::AddPathFlag(const char* pszFlag)
{
	std::shared_ptr<Node> pBegin = GetCurrentNode();
	std::shared_ptr<Node> pEnd = GetAimNode(pBegin);

	if (!CheckNodes(pBegin, pEnd, true))
		return;

	int flag = GetFlagsByName(pszFlag);
	if (!CheckFlags(flag))
		return;

	std::shared_ptr<Path> pPath = pBegin->GetPath(pEnd);
	pPath->SetFlags(pPath->GetFlags() | flag);

	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Add flags '%s' for path #%d\n", pszFlag, PATH_INDEX(pPath)));
}

void Editor::RemoveNodeFlag(const char* pszFlag)
{
	std::shared_ptr<Node> pNode = GetCurrentNode();
	if (!CheckNode(pNode))
		return;

	int flag = GetFlagsByName(pszFlag);
	if (!CheckFlags(flag))
		return;

	pNode->SetFlags(pNode->GetFlags() & ~flag);
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Remove flags '%s' for node #%d\n", pszFlag, NODE_INDEX(pNode)));
}

void Editor::RemovePathFlag(const char *pszFlag)
{
	std::shared_ptr<Node> pBegin = GetCurrentNode();
	std::shared_ptr<Node> pEnd = GetAimNode(pBegin);

	if (!CheckNodes(pBegin, pEnd, true))
		return;

	int flag = GetFlagsByName(pszFlag);
	if (!CheckFlags(flag))
		return;

	std::shared_ptr<Path> pPath = pBegin->GetPath(pEnd);
	pPath->SetFlags(pPath->GetFlags() & ~flag);

	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Remove flags '%s' for path #%d\n", pszFlag, PATH_INDEX(pPath)));
}

void Editor::SetNodeFlag(const char* pszFlag)
{
	std::shared_ptr<Node> pNode = GetCurrentNode();
	if (!CheckNode(pNode))
		return;

	int flag = GetFlagsByName(pszFlag);
	if (!CheckFlags(flag))
		return;

	pNode->SetFlags(flag);
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Set flags '%s' for node #%d\n", pszFlag, NODE_INDEX(pNode)));
}

void Editor::SetPathFlag(const char* pszFlag)
{
	std::shared_ptr<Node> pBegin = GetCurrentNode();
	std::shared_ptr<Node> pEnd = GetAimNode(pBegin);

	if (!CheckNodes(pBegin, pEnd, true))
		return;

	int flag = GetFlagsByName(pszFlag);
	if (!CheckFlags(flag))
		return;

	std::shared_ptr<Path> pPath = pBegin->GetPath(pEnd);
	pPath->SetFlags(flag);

	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Set flags '%s' for path #%d\n", pszFlag, PATH_INDEX(pPath)));
}

void Editor::SetAutoNode(float dist)
{
	if (dist == 0.0f)
		dist = AUTO_NODE_DIST;

	m_AutoNodeDist = dist;
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Set auto node distance to '%.f'\n", dist));
}

void Editor::SetAutoPath(float dist)
{
	if (dist == 0.0f)
		dist = AUTO_PATH_DIST;

	m_AutoPathDist = dist;
	CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Set auto path distance to '%.f'\n", dist));
}

void Editor::SaveFile()
{
	if (!GetNodeManager().SaveFile(m_szFilePath))
	{
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] Failed to save file.\n");
	}
	else
	{
		CLIENT_PRINTF(m_pEditor, print_console, UTIL_VarArgs("[WP] Save %d nodes and %d paths to the map file.\n",
			GetNodeManager().GetNodeSize(), GetNodeManager().GetPathSize()));
	}
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

int Editor::GetFlagsByName(const char* pszName)
{
	int flags = 0;

	std::istringstream ss(pszName);
	std::string token;
	while (std::getline(ss, token, '|'))
	{
		switch (str2int(token.c_str()))
		{
		case str2int("jump"):
			flags |= WP_FL_JUMP;
			break;
		case str2int("duck"):
			flags |= WP_FL_DUCK;
			break;
		case str2int("ladder"):
			flags |= WP_FL_LADDER;
			break;
		default:
			int num = std::stoi(token);
			if (num >= 1 && num <= 32)
			{
				num--;
				flags |= (1 << num);
			}
			break;
		}
	}

	return flags;
}

std::string Editor::GetNameByFlags(int flags)
{
	std::string str;

	for (int i = 0; i < 32; i++)
	{
		if (flags & (1 << i))
		{
			if (!str.empty())
				str += ",";

			switch (i)
			{
			case 0:
				str += "jump";
				break;
			case 1:
				str += "duck";
				break;
			case 2:
				str += "ladder";
				break;
			default:
				str += std::to_string(i + 1);
			}
		}
	}

	return str;
}

bool Editor::CheckNodes(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, bool path)
{
	if (pBegin == nullptr || pEnd == nullptr || pBegin == pEnd)
	{
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] Please standing on a node and aiming to the another node.\n");
		return false;
	}

	if (path && pBegin->GetPath(pEnd) == nullptr)
	{
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] These nodes are not connected.\n");
		return false;
	}

	return true;
}

bool Editor::CheckNode(std::shared_ptr<Node> pNode)
{
	if (pNode == nullptr)
	{
		CLIENT_PRINTF(m_pEditor, print_console, "[WP] Please standing on a target node.\n");
		return false;
	}

	return true;
}

bool Editor::IsConnectable(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, float distance)
{
	if (pBegin == nullptr || pEnd == nullptr || pBegin == pEnd)
		return false;

	// close enough to connect
	if ((pBegin->GetPosition() - pEnd->GetPosition()).Length() <= distance)
	{
		// is freesss
		if (UTIL_IsVisible(pBegin->GetPosition(), pEnd->GetPosition(), NULL, ignore_monsters))
		{
			return true;
		}
	}

	return false;
}

void Editor::AutoCreateNode(edict_t* pEntity, const Vector &origin, float min_dist, int flags, bool check_dist, bool off_node)
{
	std::shared_ptr<Node> pLastNode = m_pLastNode.lock();

	if (off_node)
	{
		if (pLastNode == nullptr)
			return;

		float height = pLastNode->GetPosition().z - origin.z;
		if ((origin - pLastNode->GetPosition()).Length() < min_dist || ((!m_IsJumping && (height < AUTO_MIN_HEIGHT || height > AUTO_MAX_HEIGHT))))
		{
			if (m_IsNewNode)
			{
				GetNodeManager().RemoveNode(pLastNode);
				m_pLastNode = std::weak_ptr<Node>();
			}

			return;
		}
	}

	std::shared_ptr<Node> pClosest;
	float dist = GetNodeManager().GetClosestNode(origin, pClosest, 1024.0f, false);

	if (check_dist && dist < m_AutoNodeDist && dist > min_dist)
		return;

	bool is_new_node = false;
	std::shared_ptr<Node> pNewNode = pClosest;

	if (dist > min_dist || pClosest == nullptr)
	{
		int f = (pEntity->v.flags & FL_DUCKING) ? WP_FL_DUCK : 0;
		if (flags != 0)
			f |= flags;

		pNewNode = GetNodeManager().CreateNode(origin, f);
		is_new_node = true;
	}

	float max_dist = m_AutoPathDist;

	if (off_node)
		max_dist = m_IsJumping ? AUTO_MAX_JUMP_DIST : AUTO_MAX_DIST;

	bool connectable = false;

	if (pLastNode != nullptr && IsConnectable(pLastNode, pNewNode, max_dist) && (off_node || m_IsStillOnGround))
	{
		std::shared_ptr<Path> pPath = pLastNode->GetPath(pNewNode);
		if (pPath == nullptr)
			pPath = GetNodeManager().CreatePath(pLastNode, pNewNode, m_IsJumping ? WP_FL_JUMP : 0);
		
		if (off_node && m_IsJumping)
		{
			pLastNode->SetFlags(pLastNode->GetFlags() | WP_FL_JUMP);
			pPath->SetFlags(pPath->GetFlags() | WP_FL_JUMP);
		}

		Vector mid = (pNewNode->GetPosition() + pLastNode->GetPosition()) * 0.5;

		TraceResult tr;
		TRACE_LINE(mid, Vector(mid.x, mid.y, mid.z - 1024.0f), ignore_monsters, pEntity, &tr);

		if (!off_node || m_IsJumping || mid.z - tr.vecEndPos.z <= 45.0f)
			GetNodeManager().CreatePath(pNewNode, pLastNode, 0);

		connectable = true;
	}
	else if (off_node)
	{
		if (is_new_node)
			GetNodeManager().RemoveNode(pNewNode);

		if (m_IsNewNode)
			GetNodeManager().RemoveNode(pLastNode);

		m_pLastNode = std::weak_ptr<Node>();
	}

	if (!off_node || connectable)
	{
		m_pLastNode = pNewNode;
		m_IsStillOnGround = pEntity->v.flags & FL_ONGROUND ? true : false;
	}

	m_IsNewNode = is_new_node;
}

std::shared_ptr<Node> Editor::GetCurrentNode() const
{
	std::shared_ptr<Node> pNode;
	GetNodeManager().GetClosestNode(m_pEditor->v.origin, pNode, 50, true);

	return pNode;
}

std::shared_ptr<Node> Editor::GetAimNode(std::shared_ptr<Node> pSkip) const
{
	return GetNodeManager().GetAimNode(m_pEditor, 16, 1000, pSkip);
}

void Editor::DrawNode(std::shared_ptr<Node> pNode, bool is_current, bool is_aiming) const
{
	Vector pos = pNode->GetPosition();

	byte color[3] = {0, 255, 0};
	float offset[2] = {-36.0f, 36.0f};
	float brightness = 250;
	int width = 10;

	if (pNode->GetFlags() & WP_FL_DUCK)
	{
		offset[0] = -18.0f;
		offset[1] = 32.0f;
	}

	if (is_current)
	{
		color[0] = 255;
		color[1] = 0;
		color[2] = 0;
		width = 15;
	}
	else if (is_aiming)
	{
		color[0] = 100;
		color[1] = 100;
		color[2] = 100;
		brightness = 150;
		width = 15;

		UTIL_BeamPoints(m_pEditor,
			m_pEditor->v.origin, pos,
			s_spr_arrow1, 0, 0,
			5, 10, 0,
			200, 200, 200, 200,
			0);
	}
	else if (pNode->GetFlags() & WP_FL_JUMP)
	{
		color[0] = 250;
		color[1] = 0;
		color[2] = 200;
	}
	else if (pNode->GetFlags() & WP_FL_LADDER)
	{
		color[0] = 0;
		color[1] = 200;
		color[2] = 200;
	}

	UTIL_BeamPoints(m_pEditor,
		Vector(pos.x, pos.y, pos.z + offset[0]),
		Vector(pos.x, pos.y, pos.z + offset[1]),
		s_spr_zbeam4, 0, 0,
		7, width, 0,
		color[0], color[1], color[2], brightness,
		0);

	//SERVER_PRINT(UTIL_VarArgs("#%d..", NODE_INDEX(pNode)));
}

void Editor::DrawPath(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd) const
{
	byte color[3] = {200, 200, 0};

	std::shared_ptr<Path> pPath = pBegin->GetPath(pEnd);
	if (pPath != nullptr)
	{
		if (pPath->GetFlags() & WP_FL_JUMP)
		{
			color[0] = 200;
			color[1] = 0;
			color[2] = 200;
		}
		else if (pPath->GetFlags() & WP_FL_LADDER)
		{
			color[0] = 0;
			color[1] = 200;
			color[2] = 200;
		}
		else if (pEnd->GetPath(pBegin) == nullptr)
		{
			color[0] = 200;
			color[1] = 100;
			color[2] = 0;
		}
	}
	else if (pEnd->GetPath(pBegin) != nullptr)
	{
		color[0] = 255;
		color[1] = 0;
		color[2] = 0;
	}

	UTIL_BeamPoints(m_pEditor,
		pBegin->GetPosition(),
		pEnd->GetPosition(),
		s_spr_zbeam1, 0, 0, 7, 10, 3,
		color[0], color[1], color[2], 250, 0);
}

Editor& GetEditor() 
{
	return s_Editor;
}
