#include "amxxapi.h"
#include "amxxmodule.h"
#include "Waypoint.h"
#include "Node.h"
#include "Path.h"

bool WP_FindShortestPath(Node* pStart, Node* pGoal, std::vector<Node*>& result)
{
	if (pStart != nullptr && pGoal != nullptr)
	{
		if (g_PathFinder.CalcPath(pStart, pGoal, result))
		{
			return true;
		}
	}

	return false;
}

Node* WP_GetClosestNode(const Vector& origin, float radius, bool visible)
{
	return g_NodeManager.GetClosestNode(origin, radius, visible);
}

void WP_RegisterFunctions()
{
	MF_RegisterFunctionEx(&WP_FindShortestPath, "WP_FindShortestPath");
	MF_RegisterFunctionEx(&WP_GetClosestNode, "WP_GetClosestNode");
}