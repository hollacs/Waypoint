#pragma once

#ifdef WAYPOINT_EXPORTS
#  define WAYPOINT_API __declspec(dllexport)
#else
#  define WAYPOINT_API __declspec(dllimport)
#endif

#include <vector>

#include "Node.h"

typedef std::vector<std::weak_ptr<Node>> PathResult;

class PathFinder
{
public:
	WAYPOINT_API bool CalcPath(std::shared_ptr<Node> pStart, std::shared_ptr<Node> pGoal, PathResult&result);

private:
	struct PF_Node
	{
		PF_Node(std::shared_ptr<Node> pNode, bool open, float f, float g)
		{
			m_f = f;
			m_g = g;
			m_open = open;
			m_pParent = nullptr;
			m_pNode = pNode;
		}

		PF_Node()
		{
			m_f = 0.0f;
			m_g = 0.0f;
			m_open = false;
			m_pParent = nullptr;
			m_pNode = nullptr;
		}

		float m_f, m_g;
		bool m_open;
		PF_Node* m_pParent;
		std::shared_ptr<Node> m_pNode;
	};

	PathResult ReconstructPath(PF_Node *pCurrent);
};

WAYPOINT_API PathFinder& GetPathFinder();