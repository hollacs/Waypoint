#include "WaypointApi.h"

bool wp_GetShortestPath(std::shared_ptr<Node> pStart, std::shared_ptr<Node> pGoal, PathResult& result)
{
	std::weak_ptr<Node> pBegin = pStart;
	std::weak_ptr<Node> pEnd = pGoal;

	if (pBegin.expired() || pEnd.expired())
		return false;

	return GetPathFinder().CalcPath(pStart, pGoal, result);
}