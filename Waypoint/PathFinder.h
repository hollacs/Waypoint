#pragma once

#include <vector>

class Node;

class PathFinder
{
public:
	bool CalcPath(Node* pStart, Node* pGoal, std::vector<Node*> &result);

private:
	struct PF_Node
	{
		PF_Node(Node* pNode, bool open, float f, float g)
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
		Node* m_pNode;
	};

	std::vector<Node*> ReconstructPath(PF_Node* pCurrent);
};