#pragma once

#include "PathFinder.h"

bool wp_GetShortestPath(std::shared_ptr<Node> pStart, std::shared_ptr<Node> pGoal, PathResult& result);