#include "WaypointApi.h"
#include "amxxmodule.h"

PFN_FIND_CLOSEST_NODE g_fn_FindClosestNode;
PFN_FIND_SHORTEST_PATH g_fn_FindShortestPath;

void WP_RequestFunctions()
{
	g_fn_FindClosestNode = (PFN_FIND_CLOSEST_NODE)MF_RequestFunction("WP_GetClosestNode");
	g_fn_FindShortestPath = (PFN_FIND_SHORTEST_PATH)MF_RequestFunction("WP_FindShortestPath");
}