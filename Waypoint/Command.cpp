#include "Command.h"
#include "meta_api.h"
#include "Waypoint.h"
#include <vector>
#include "Node.h"
#include "Path.h"
#include "Utils.h"

edict_t* g_pEditor;
bool g_AutoMode;
float g_NodeDist, g_PathDist;
float g_EditorTime;
float g_AutoTime;
Node* g_pLastNode;
Node* g_pStartNode;
Node* g_pEndNode;
int g_IsJumping;
float g_FallVelocity;

void EditorInit()
{
	g_pEditor = nullptr;
	g_EditorTime = 0.0f;
	g_AutoTime = 0.0f;
	g_AutoMode = false;
	g_NodeDist = 120.0f;
	g_PathDist = 180.0f;
	g_pLastNode = nullptr;
	g_pStartNode = nullptr;
	g_pEndNode = nullptr;
	g_IsJumping = 0;
	g_FallVelocity = 0.0f;
}

void EditorCommands(edict_t *pClient)
{
	const char *pszCmd = CMD_ARGV(0);

	if (strcmp(pszCmd, "wp") != 0)
		RETURN_META(MRES_IGNORED);

	pszCmd = CMD_ARGV(1);

	if (strcmp(pszCmd, "edit") == 0)
	{
		int value = atoi(CMD_ARGV(2));
		if (value == 1)
		{
			g_pEditor = pClient;
			CLIENT_PRINTF(pClient, print_console, "[WP] Edit mode has been enabled.\n");
		}
		else if (value == 0)
		{
			g_pEditor = nullptr;
		 	CLIENT_PRINTF(pClient, print_console, "[WP] Edit mode has been disabled.\n");
		}
	}

	if (g_pEditor != pClient)
	{
		CLIENT_PRINTF(pClient, print_console, "[WP] You are not in the edit mode.\n");
		RETURN_META(MRES_SUPERCEDE);
	}

	if (strcmp(pszCmd, "create") == 0)
	{
		const char* pszArg = CMD_ARGV(2);

		if (strcmp(pszArg, "node") == 0)
		{
			int flags = 0;
			pszArg = CMD_ARGV(3);
			if (pszArg[0])
				flags = atoi(pszArg);

			g_NodeManager.CreateNode(pClient->v.origin, flags);

			CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Create node #%d\n", g_NodeManager.GetNodeSize()));
		}
		else if (strcmp(pszArg, "path") == 0)
		{
			Node* pBegin = g_NodeManager.GetClosestNode(pClient->v.origin, 50);
			Node* pEnd = g_NodeManager.GetAimNode(pClient, 16, 1000, pBegin);

			if (pBegin == nullptr || pEnd == nullptr || pBegin == pEnd)
			{
				CLIENT_PRINTF(pClient, print_console, "[WP] You must standing on a node and aiming to another node.\n");
			}
			else
			{
				int mode = 0;
				pszArg = CMD_ARGV(3);
				if (pszArg[0])
					mode = atoi(pszArg);

				int flags = 0;
				pszArg = CMD_ARGV(4);
				if (pszArg[0])
					flags = atoi(pszArg);

				if (mode == 0)
				{
					g_NodeManager.CreatePath(pBegin, pEnd, flags);
					g_NodeManager.CreatePath(pEnd, pBegin, flags);

					CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Create bothway path to connect nodes (#%d <--> #%d)\n", g_NodeManager.GetNodeIndex(pBegin), g_NodeManager.GetNodeIndex(pEnd)));
				}
				else if (mode == 1)
				{
					g_NodeManager.CreatePath(pBegin, pEnd, flags);
					
					Path* pPath = pBegin->GetPath(pEnd);
					g_NodeManager.RemovePath(pPath);

					CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Create outgoing path to connect nodes (#%d --> #%d)\n", g_NodeManager.GetNodeIndex(pBegin), g_NodeManager.GetNodeIndex(pEnd)));
				}
				else if (mode == 2)
				{
					g_NodeManager.CreatePath(pEnd, pBegin, flags);

					Path* pPath = pEnd->GetPath(pBegin);
					g_NodeManager.RemovePath(pPath);

					CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Create incoming path to connect nodes (#%d <-- #%d)\n", g_NodeManager.GetNodeIndex(pBegin), g_NodeManager.GetNodeIndex(pEnd)));
				}
			}
		}
	}
	else if (strcmp(pszCmd, "remove") == 0)
	{
		const char *pszArg = CMD_ARGV(2);

		if (strcmp(pszArg, "node") == 0)
		{
			Node* pNode = g_NodeManager.GetClosestNode(pClient->v.origin, 50);
			if (pNode == nullptr)
			{
				CLIENT_PRINTF(pClient, print_console, "[WP] Couldn't find any node nearby you.\n");
			}
			else
			{
				if (g_pLastNode == pNode)
					g_pLastNode = nullptr;

				size_t index = g_NodeManager.GetNodeIndex(pNode);
				g_NodeManager.RemoveNode(pNode);
				CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Remove node #%d\n", index));
			}
		}
		else if (strcmp(pszArg, "path") == 0)
		{
			Node* pBegin = g_NodeManager.GetClosestNode(pClient->v.origin, 50);
			Node* pEnd = g_NodeManager.GetAimNode(pClient, 16, 1000, pBegin);

			if (pBegin == nullptr || pEnd == nullptr || pBegin == pEnd)
			{
				CLIENT_PRINTF(pClient, print_console, "[WP] You must standing on a node and aiming to another node.\n");
			}
			else if (pBegin->GetPath(pEnd) == nullptr && pEnd->GetPath(pBegin) == nullptr)
			{
				CLIENT_PRINTF(pClient, print_console, "[WP] Nodes are not connected.\n");
			}
			else
			{
				Path* pPath = pBegin->GetPath(pEnd);
				g_NodeManager.RemovePath(pPath);

				pPath = pEnd->GetPath(pBegin);
				g_NodeManager.RemovePath(pPath);

				CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Remove path for both node (#%d -- #%d)\n", g_NodeManager.GetNodeIndex(pBegin), g_NodeManager.GetNodeIndex(pEnd)));
			}
		}
	}
	else if (strcmp(pszCmd, "auto") == 0)
	{
		int value = atoi(CMD_ARGV(2));
		if (value == 1)
		{
			g_AutoMode = true;
			CLIENT_PRINTF(pClient, print_console, "[WP] Auto mode has been enabled.\n");
		}
		else if (value == 0)
		{
			g_AutoMode = false;
			CLIENT_PRINTF(pClient, print_console, "[WP] Auto mode has been disabled.\n");
		}
	}
	else if (strcmp(pszCmd, "set") == 0)
	{
		const char* pszArg = CMD_ARGV(2);

		if (strcmp(pszArg, "node") == 0)
		{
			const char* pszArg = CMD_ARGV(3);

			if (strcmp(pszArg, "dist") == 0)
			{
				float dist = 120;

				if (pszArg[0])
					dist = atof(pszArg);

				g_NodeDist = dist;
				CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Auto node creation distance set to %.f\n", g_NodeDist));
			}
		}
		else if (strcmp(pszArg, "path") == 0)
		{
			const char* pszArg = CMD_ARGV(3);

			if (strcmp(pszArg, "dist") == 0)
			{
				float dist = 180;

				if (pszArg[0])
					dist = atof(pszArg);

				g_PathDist = dist;
				CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Auto path creation distance set to %.f\n", g_PathDist));
			}
		}
	}
	else if (strcmp(pszCmd, "save") == 0)
	{
		g_NodeManager.SaveFile(g_pszFilePath);
		CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] Save %d nodes and %d paths to file.\n", g_NodeManager.GetNodeSize(), g_NodeManager.GetPathSize()));
	}
	else if (strcmp(pszCmd, "path") == 0)
	{
		const char* pszArg = CMD_ARGV(2);

		if (strcmp(pszArg, "start") == 0)
		{
			g_pStartNode = g_NodeManager.GetClosestNode(pClient->v.origin, 50);

			if (g_pStartNode != nullptr)
				CLIENT_PRINTF(pClient, print_console, "[WP] PathFinder: Start node set.\n");
		}
		else if (strcmp(pszArg, "end") == 0)
		{
			g_pEndNode = g_NodeManager.GetClosestNode(pClient->v.origin, 50);

			if (g_pEndNode != nullptr)
				CLIENT_PRINTF(pClient, print_console, "[WP] PathFinder: End node set.\n");
		}
		else if (strcmp(pszArg, "find") == 0)
		{
			if (g_pStartNode != nullptr && g_pEndNode != nullptr)
			{
				std::vector<Node*> result;
				g_PathFinder.CalcPath(g_pStartNode, g_pEndNode, result);

				if (result.size() > 1)
				{
					for (auto it1 = result.begin(), it2 = ++result.begin(); it2 != result.end(); ++it1, ++it2)
					{
						UTIL_BeamPoints(pClient, (*it1)->GetPosition(), (*it2)->GetPosition(), g_SpriteBeam4, 0, 0, 100, 10, 3, 0, 50, 250, 255, 0);
					}

					CLIENT_PRINTF(pClient, print_console, UTIL_VarArgs("[WP] PathFinder: Path found (%d)\n", result.size()));
				}
			}
		}
	}


	RETURN_META(MRES_SUPERCEDE);
}

void EditorCmdStart(const edict_t* player, const struct usercmd_s* cmd)
{
	if (g_pEditor != player)
		RETURN_META(MRES_IGNORED);

	if (g_AutoMode)
	{
		if ((cmd->buttons & IN_JUMP) && !(player->v.oldbuttons & IN_JUMP) && (player->v.flags & FL_ONGROUND))
		{
			// jump
			g_IsJumping = 1;

			Node* pNode = g_NodeManager.GetClosestNode(player->v.origin, 32);
			if (pNode == nullptr)
			{
				pNode = g_NodeManager.CreateNode(player->v.origin, (player->v.flags & FL_DUCKING) ? FL_NODE_DUCK : 0);
				CLIENT_COMMAND(const_cast<edict_t *>(player), "spk fvox/blip\n");
			}

			g_pLastNode = pNode;
			pNode->SetFlags(pNode->GetFlags() | FL_NODE_JUMP);
		}
	}
}

void EditorThink(edict_t* pEditor)
{
	if (g_pEditor != pEditor)
		RETURN_META(MRES_IGNORED);

	if (g_AutoMode)
	{
		if (pEditor->v.flags & FL_ONGROUND)
		{
			if (gpGlobals->time >= g_AutoTime + 0.1)
			{
				if (g_IsJumping)
				{
					if (g_pLastNode != nullptr)
					{
						Node* pNode = g_NodeManager.GetClosestNode(pEditor->v.origin, 32);
						if (pNode == nullptr)
						{
							pNode = g_NodeManager.CreateNode(pEditor->v.origin, (pEditor->v.flags & FL_DUCKING) ? FL_NODE_DUCK : 0);
							CLIENT_COMMAND(pEditor, "spk fvox/blip\n");
						}

						if (g_pLastNode != pNode)
						{
							Path* pPath = g_pLastNode->GetPath(pNode);
							if (pPath == nullptr)
								pPath = g_NodeManager.CreatePath(g_pLastNode, pNode, 0);

							pPath->SetFlags(pPath->GetFlags() | FL_PATH_JUMP);

							if (g_FallVelocity < 250.0f && pNode->GetPath(g_pLastNode) == nullptr)
							{
								g_NodeManager.CreatePath(pNode, g_pLastNode, 0);
								CLIENT_COMMAND(pEditor, "spk items/medshotno1\n");
							}

							g_pLastNode = pNode;
						}
					}
				}
				else if (g_FallVelocity > 200.0f)
				{
					if (g_pLastNode != nullptr)
					{
						Node* pNode = g_NodeManager.GetClosestNode(pEditor->v.origin, 48);
						if (pNode == nullptr)
						{
							pNode = g_NodeManager.CreateNode(pEditor->v.origin, (pEditor->v.flags & FL_DUCKING) ? FL_NODE_DUCK : 0);
							CLIENT_COMMAND(pEditor, "spk fvox/blip\n");
						}

						if (g_pLastNode != pNode)
						{
							if (g_pLastNode->GetPath(pNode) == nullptr)
							{
								SERVER_PRINT("fall node\n");
								g_NodeManager.CreatePath(g_pLastNode, pNode, 0);
								CLIENT_COMMAND(pEditor, "spk items/medshotno1\n");
							}

							g_pLastNode = pNode;
						}
					}
				}
				else
				{
					Node* pNode = g_NodeManager.GetClosestNode(pEditor->v.origin, 32);

					if (g_pLastNode == nullptr)
					{
						if (pNode != nullptr)
							g_pLastNode = pNode;
					}
					else
					{
						if (pNode != nullptr)
						{
							if ((g_pLastNode->GetPosition() - pNode->GetPosition()).Length() < g_PathDist
								&& UTIL_IsVisible(g_pLastNode->GetPosition(), pNode->GetPosition(), pEditor, ignore_monsters))
							{
								if (g_pLastNode->GetPath(pNode) == nullptr || pNode->GetPath(g_pLastNode) == nullptr)
								{
									g_NodeManager.CreatePath(g_pLastNode, pNode, 0);
									g_NodeManager.CreatePath(pNode, g_pLastNode, 0);
									CLIENT_COMMAND(pEditor, "spk items/medshotno1\n");
								}
							}

							g_pLastNode = pNode;
						}
						else
						{
							float distance = (g_pLastNode->GetPosition() - pEditor->v.origin).Length();
							if (distance >= g_NodeDist)
							{
								Node* pNode = g_NodeManager.GetClosestNode(pEditor->v.origin, 72);
								if (pNode == nullptr)
								{
									pNode = g_NodeManager.CreateNode(pEditor->v.origin, (pEditor->v.flags & FL_DUCKING) ? FL_NODE_DUCK : 0);
									CLIENT_COMMAND(pEditor, "spk fvox/blip\n");

									if (distance < g_PathDist && UTIL_IsVisible(g_pLastNode->GetPosition(), pNode->GetPosition(), pEditor, ignore_monsters))
									{
										if (g_pLastNode->GetPath(pNode) == nullptr || pNode->GetPath(g_pLastNode) == nullptr)
										{
											g_NodeManager.CreatePath(g_pLastNode, pNode, 0);
											g_NodeManager.CreatePath(pNode, g_pLastNode, 0);
											CLIENT_COMMAND(pEditor, "spk items/medshotno1\n");
										}
									}

									g_pLastNode = pNode;
								}
								else if (distance > g_PathDist)
								{
									g_pLastNode = nullptr;
								}
							}
						}
					}
				}

				g_FallVelocity = 0.0f;
				g_IsJumping = false;
				g_AutoTime = gpGlobals->time;
			}
		}
		else
		{
			g_FallVelocity = pEditor->v.flFallVelocity;
		}
	}

	if (gpGlobals->time >= g_EditorTime + 0.5)
	{
		auto nodes = g_NodeManager.GetNodes();
		if (nodes.empty())
			RETURN_META(MRES_IGNORED);

		size_t max_nodes = min(nodes.size(), 40);

		Node* pCurrentNode = nullptr;

		for (size_t i = 0; i < max_nodes; i++)
		{
			Node *pNode = nullptr;
			float min_distance = 1000;
			std::vector<Node*>::iterator node_iter;

			for (auto iter = nodes.begin(); iter != nodes.end(); iter++)
			{
				float distance = ((*iter)->GetPosition() - pEditor->v.origin).Length();
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

			Vector pos = pNode->GetPosition();

			int color[3] = { 0, 0, 0 };

			if (i == 0 && min_distance <= 50)
			{
				pCurrentNode = pNode;

				const auto &paths = pNode->GetPaths();

				for (auto iter = paths.begin(); iter != paths.end(); iter++)
				{
					int color[3] = {0, 0, 0};

					Node* pBeginNode = pNode;
					Node* pEndNode = (*iter)->GetEnd();

					Path* pPath = pBeginNode->GetPath(pEndNode);
					if (pPath != nullptr && (pPath->GetFlags() & FL_PATH_JUMP))
					{
						color[0] = 200;
						color[1] = 0;
						color[2] = 200;
					}
					else
					{
						if (pEndNode->GetPath(pBeginNode) != nullptr)
						{
							color[0] = 200;
							color[1] = 200;
							color[2] = 0;
						}
						else
						{
							color[0] = 200;
							color[1] = 100;
							color[2] = 0;
						}
					}

					Vector pos2 = pEndNode->GetPosition();

					UTIL_BeamPoints(pEditor,
						pos, pos2,
						g_SpriteBeam1, 0, 0,
						5, 10, 3,
						color[0], color[1], color[2], 255,
						0);
				}

				const auto &nodes = g_NodeManager.GetNodes();

				for (auto iter = nodes.begin(); iter != nodes.end(); iter++)
				{
					Node* pBeginNode = pNode;
					Node* pEndNode = *iter;

					if (pEndNode->GetPath(pBeginNode) != nullptr && pBeginNode->GetPath(pEndNode) == nullptr)
					{
						Vector pos2 = pEndNode->GetPosition();

						UTIL_BeamPoints(pEditor,
							pos, pos2,
							g_SpriteBeam1, 0, 0,
							5, 10, 3,
							255, 0, 0, 255,
							0);
					}
				}

				color[0] = 255;
			}
			else
			{
				color[1] = 255;
			}

			float offset[2];

			if (pNode->GetFlags() & FL_NODE_DUCK)
			{
				offset[0] = -18;
				offset[1] = 8;
			}
			else
			{
				offset[0] = -36;
				offset[1] = 16;
			}

			UTIL_BeamPoints(pEditor,
				Vector(pos.x, pos.y, pos.z + offset[0]),
				Vector(pos.x, pos.y, pos.z + offset[1]),
				g_SpriteBeam4, 0, 0,
				5, 10, 0,
				color[0], color[1], color[2], 255,
				0);

			if (pNode->GetFlags() & FL_NODE_JUMP)
			{
				if (pNode->GetFlags() & FL_NODE_DUCK)
				{
					offset[0] = 8;
					offset[1] = 16;
				}
				else
				{
					offset[0] = 16;
					offset[1] = 32;
				}

				UTIL_BeamPoints(pEditor,
					Vector(pos.x, pos.y, pos.z + offset[0]),
					Vector(pos.x, pos.y, pos.z + offset[1]),
					g_SpriteBeam4, 0, 0,
					5, 10, 0,
					200, 0, 200, 255,
					0);
			}
		}

		Node* pAimNode = g_NodeManager.GetAimNode(pEditor, 16, 1000, pCurrentNode);
		if (pAimNode != nullptr)
		{
			Vector pos = pAimNode->GetPosition();

			UTIL_BeamPoints(pEditor,
				Vector(pos.x, pos.y, pos.z - 36),
				Vector(pos.x, pos.y, pos.z + 16),
				g_SpriteBeam4, 0, 0,
				7, 10, 0,
				255, 255, 255, 255,
				0);

			UTIL_BeamPoints(pEditor,
				pEditor->v.origin, pos,
				g_SpriteArrow, 0, 0,
				5, 10, 0,
				255, 255, 255, 255,
				0);
		}

		g_EditorTime = gpGlobals->time;
	}

	RETURN_META(MRES_IGNORED);
}

void EditorReset()
{
	g_pEditor = nullptr;
	g_AutoMode = false;
	g_EditorTime = 0.0f;
	g_AutoTime = 0.0f;
	g_pLastNode = nullptr;
	g_pStartNode = nullptr;
	g_pEndNode = nullptr;
	g_IsJumping = 0;
	g_FallVelocity = 0.0f;
}