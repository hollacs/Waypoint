#include "PathFinder.h"
#include "Node.h"
#include "Path.h"

#include <queue>
#include <map>

std::vector<Node*> PathFinder::ReconstructPath(PF_Node* pCurrent)
{
    std::vector<Node*> result;
    PF_Node* pParent = pCurrent;

    while (pParent != nullptr)
    {
        result.insert(result.begin(), pParent->m_pNode);
        pParent = pParent->m_pParent;
    }

    return result;
}

bool PathFinder::CalcPath(Node* pStart, Node* pGoal, std::vector<Node*> &result)
{
    auto cmp = [](const PF_Node* pLeft, const PF_Node* pRight) { return pLeft->m_f > pRight->m_f; };
    std::priority_queue<PF_Node*, std::vector<PF_Node*>, decltype(cmp)> open_list(cmp);
    std::map<Node*, PF_Node> node_map;

    node_map[pStart] = PF_Node(pStart, true, (pStart->GetPosition() - pGoal->GetPosition()).Length(), 0.0f);
    open_list.push(&node_map[pStart]);

    while (!open_list.empty())
    {
        PF_Node* pCurrent = open_list.top();

        if (pCurrent->m_pNode == pGoal)
        {
            result = ReconstructPath(pCurrent);
            return true;
        }

        open_list.pop();

        const auto &paths = pCurrent->m_pNode->GetPaths();

        for (auto p : paths)
        {
            Node* pChild = p->GetEnd();
            PF_Node *pNode = nullptr;

            float g = g = pCurrent->m_g + (pCurrent->m_pNode->GetPosition() - pChild->GetPosition()).Length();

            auto iter = node_map.find(pChild);
            if (iter != node_map.end())
            {
                pNode = &iter->second;

                if (g >= pNode->m_g)
                    continue;
            }
            else
            {
                node_map[pChild] = PF_Node(pChild, false, 0.0f, 0.0f);
                pNode = &node_map[pChild];
            }

            pNode->m_g = g;
            pNode->m_f = g + (pChild->GetPosition() - pGoal->GetPosition()).Length();
            pNode->m_pParent = pCurrent;

            if (!pNode->m_open)
            {
                pNode->m_open = true;
                open_list.push(pNode);
            }
        }
    }

    return false;
}
