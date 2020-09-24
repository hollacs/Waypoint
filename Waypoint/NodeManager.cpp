#include <algorithm>
#include <sstream>
#include "NodeManager.h"
#include "meta_api.h"

#include "Node.h"
#include "Path.h"
#include "Utils.h"

Node* NodeManager::CreateNode(const Vector& origin, int flags)
{
	Node* pNode = new Node(origin, flags);

	m_nodes.push_back(pNode);
	return pNode;
}

void NodeManager::RemoveNode(Node* pRemoveNode)
{
	for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		Node* pNode = *it;
		Path* pPath = pNode->GetPath(pRemoveNode);

		if (pPath != nullptr)
		{
			RemovePath(pPath);
		}
	}

	const auto& paths = pRemoveNode->GetPaths();

	for (auto it = paths.begin(); it != paths.end(); it++)
	{
		m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), *it), m_paths.end());
		delete *it;
	}

	m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), pRemoveNode), m_nodes.end());
	delete pRemoveNode;
}

Node* NodeManager::GetNodeAt(size_t index) const
{
	return m_nodes.at(index);
}

int NodeManager::GetNodeIndex(Node* pNode) const
{
	auto iter = std::find(m_nodes.begin(), m_nodes.end(), pNode);
	if (iter == m_nodes.end())
		return -1;

	size_t index = std::distance(m_nodes.begin(), iter);
	return index;
}

Node* NodeManager::GetClosestNode(const Vector& origin, float radius, bool visible) const
{
	Node* pBestNode = nullptr;
	float min_distance = radius;

	for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		Node* pNode = *it;

		float distance = (pNode->GetPosition() - origin).Length();
		if (distance < min_distance)
		{
			if (visible && UTIL_IsVisible(origin, pNode->GetPosition(), NULL, ignore_monsters))
			{
				pBestNode = pNode;
				min_distance = distance;
			}
		}
	}

	return pBestNode;
}

Node* NodeManager::GetClosestNodeInView(const Vector& origin, const Vector& angle, float radius, float fov, bool visible, bool use_2d) const
{
	Node* pBestNode = nullptr;
	float min_distance = radius;

	for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		Node* pNode = *it;

		float distance = (pNode->GetPosition() - origin).Length();
		if (distance < min_distance)
		{
			if (UTIL_IsInViewCone(origin, angle, pNode->GetPosition(), fov, use_2d))
			{
				if (visible && UTIL_IsVisible(origin, pNode->GetPosition(), NULL, ignore_monsters))
				{
					pBestNode = pNode;
					min_distance = distance;
				}
			}
		}
	}

	return pBestNode;
}

Path* NodeManager::CreatePath(Node* pBegin, Node* pEnd, int flags)
{
	if (pBegin->GetPath(pEnd) != nullptr)
		return nullptr;

	Path* pPath = new Path(pBegin, pEnd, flags);

	m_paths.push_back(pPath);
	pBegin->AddPath(pPath);

	return pPath;
}

void NodeManager::RemovePath(Path* pPath)
{
	if (pPath == nullptr)
		return;

	pPath->GetBegin()->RemovePath(pPath);
	m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), pPath), m_paths.end());
	delete pPath;
}

Path* NodeManager::GetPathAt(size_t index) const
{
	return m_paths.at(index);
}

int NodeManager::GetPathIndex(Path* pPath) const
{
	auto iter = std::find(m_paths.begin(), m_paths.end(), pPath);
	if (iter == m_paths.end())
		return -1;

	size_t index = std::distance(m_paths.begin(), iter);
	return index;
}

void NodeManager::Clear()
{
	for (auto it = m_paths.begin(); it != m_paths.end(); it++)
	{
		delete *it;
	}

	for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		delete *it;
	}

	m_paths.clear();
	m_nodes.clear();
}

Node* NodeManager::GetAimNode(edict_t* pEntity, float width, float max_radius, Node *pIgnoreNode) const
{
	Vector start = pEntity->v.origin + pEntity->v.view_ofs;
	Vector end;

	MAKE_VECTORS(pEntity->v.v_angle);
	end = start + (gpGlobals->v_forward * 4096);

	Node* pAimNode = nullptr;
	float min_distance = width;

	for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
	{
		Node* pCurrentNode = *it;

		if (pCurrentNode == pIgnoreNode)
			continue;

		Vector pos = pCurrentNode->GetPosition();

		if ((pos - pEntity->v.origin).Length() > max_radius)
			continue;

		float distance = UTIL_DistLineSegments(start, end, Vector(pos.x, pos.y, pos.z - 16), Vector(pos.x, pos.y, pos.z + 16));
		if (distance < min_distance)
		{
			min_distance = distance;
			pAimNode = pCurrentNode;
		}
	}

	return pAimNode;
}

Node* NodeManager::GetFacingNode(const Vector& origin, const Vector& direction) const
{
	return nullptr;
}

void NodeManager::SaveFile(const char* pszFile)
{
	FILE* pFile = fopen(pszFile, "w");
	if (pFile)
	{
		fputs("[node]\n", pFile);

		for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
		{
			Node* pNode = *it;
			Vector pos = pNode->GetPosition();

			fprintf(pFile, "%f %f %f %d\n", pos.x, pos.y, pos.z, pNode->GetFlags());
		}

		fputs("[path]\n", pFile);

		for (auto it = m_paths.begin(); it != m_paths.end(); it++)
		{
			Path* pPath = *it;
			Node* pBegin = pPath->GetBegin();
			Node* pEnd = pPath->GetEnd();

			fprintf(pFile, "%d %d %d\n", GetNodeIndex(pBegin), GetNodeIndex(pEnd), pPath->GetFlags());
		}

		fclose(pFile);
	}
}

void NodeManager::LoadFile(const char* pszFile)
{
	FILE* pFile = fopen(pszFile, "r");
	if (pFile != NULL)
	{
		this->Clear();

		char szLine[512];
		int section = 0;

		while (fgets(szLine, sizeof(szLine), pFile))
		{
			if (strlen(szLine) < 3)
				continue;

			if (strncmp(szLine, "[node]", 6) == 0)
			{
				section = 1;
				continue;
			}

			if (strncmp(szLine, "[path]", 6) == 0)
			{
				section = 2;
				continue;
			}

			if (section == 1)
			{
				Vector pos;
				int flags = 0;

				std::stringstream ss(szLine);
				ss >> pos.x >> pos.y >> pos.z >> flags;

				this->CreateNode(pos, flags);
			}
			else if (section == 2)
			{
				size_t begin_index = 0, end_index = 0;
				int flags = 0;

				std::stringstream ss(szLine);
				ss >> begin_index >> end_index >> flags;

				Node* pBegin = GetNodeAt(begin_index);
				Node* pEnd = GetNodeAt(end_index);

				if (pBegin != nullptr && pEnd != nullptr)
					this->CreatePath(pBegin, pEnd, flags);
			}
		}

		fclose(pFile);
	}
}