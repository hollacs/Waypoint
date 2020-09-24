#pragma once

#include <vector>

#include "..\Node.h"
#include "..\Path.h"

typedef Node*	(*PFN_FIND_CLOSEST_NODE)	(const Vector&, float, bool);
typedef bool	(*PFN_FIND_SHORTEST_PATH)	(Node*, Node*, std::vector<Node*>&);

extern PFN_FIND_CLOSEST_NODE g_fn_FindClosestNode;
extern PFN_FIND_SHORTEST_PATH g_fn_FindShortestPath;

#define WP_FindClosestNode g_fn_FindClosestNode
#define WP_FindShortestPath g_fn_FindShortestPath

void WP_RequestFunctions();