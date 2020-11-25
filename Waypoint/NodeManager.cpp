#include "NodeManager.h"

#include <algorithm>
#include <sstream>

#include "meta_api.h"

#include "Node.h"
#include "Path.h"
#include "Utils.h"

static NodeManager s_NodeManager;

NodeManager& GetNodeManager()
{
	return s_NodeManager;
}

std::shared_ptr<Node> NodeManager::CreateNode(const Vector& origin, int flags)
{
	auto pNode = std::make_shared<Node>(origin, flags);

	m_nodes.push_back(pNode);

	return pNode;
}

void NodeManager::RemoveNode(std::shared_ptr<Node> pRemoveNode)
{
	if (pRemoveNode == nullptr)
		return;

	for (auto it = m_paths.begin(); it != m_paths.end(); )
	{
		std::shared_ptr<Path> pPath = *it;
		if (pPath->GetEnd() == pRemoveNode)
		{
			pPath->GetBegin()->RemovePath(pPath);
			it = m_paths.erase(it);
		}
		else
		{
			it++;
		}
	}

	auto paths = pRemoveNode->GetPaths();
	for (auto it = paths.begin(); it != paths.end();)
	{
		this->RemovePath(*it);
		it = paths.erase(it);
	}

	m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), pRemoveNode), m_nodes.end());
}

std::shared_ptr<Node> NodeManager::GetNodeAt(size_t index)
{
	return m_nodes.at(index);
}

int NodeManager::GetNodeIndex(std::shared_ptr<Node> pNode) const
{
	auto iter = std::find(m_nodes.begin(), m_nodes.end(), pNode);
	if (iter == m_nodes.end())
		return -1;

	return iter - m_nodes.begin();
}

size_t NodeManager::GetNodeSize() const
{
	return m_nodes.size();
}

const std::vector<std::shared_ptr<Node>>& NodeManager::GetNodes() const
{
	return m_nodes;
}

std::vector<std::shared_ptr<Node>>& NodeManager::GetNodes()
{
	return m_nodes;
}

std::shared_ptr<Path> NodeManager::CreatePath(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, int flags)
{
	if (pBegin->GetPath(pEnd) != nullptr || pBegin == nullptr || pEnd == nullptr)
		return nullptr;

	auto pPath = std::make_shared<Path>(pBegin, pEnd, flags);
	m_paths.push_back(pPath);

	pBegin->AddPath(pPath);
	return pPath;
}

void NodeManager::RemovePath(std::shared_ptr<Path> pPath)
{
	if (pPath == nullptr)
		return;

	pPath->GetBegin()->RemovePath(pPath);
	m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), pPath), m_paths.end());
}

std::shared_ptr<Path> NodeManager::GetPathAt(size_t index) const
{
	return m_paths.at(index);
}

int NodeManager::GetPathIndex(std::shared_ptr<Path> pPath) const
{
	auto iter = std::find(m_paths.begin(), m_paths.end(), pPath);
	if (iter == m_paths.end())
		return -1;

	return iter - m_paths.begin();
}

const std::vector<std::shared_ptr<Path>>& NodeManager::GetPaths() const
{
	return m_paths;
}

std::vector<std::shared_ptr<Path>>& NodeManager::GetPaths()
{
	return m_paths;
}

size_t NodeManager::GetPathSize() const
{
	return m_paths.size();
}

void NodeManager::Clear()
{
	for (auto &pNode : m_nodes)
	{
		pNode->GetPaths().clear();
	}

	m_nodes.clear();
	m_paths.clear();
}

float NodeManager::GetClosestNode(const Vector& origin, std::shared_ptr<Node>& pResult, float radius, bool visible) const
{
	float min_distance = radius;

	for (auto& pNode : m_nodes)
	{
		float distance = (pNode->GetPosition() - origin).Length();
		if (distance < min_distance)
		{
			if (visible && !UTIL_IsVisible(origin, pNode->GetPosition(), NULL, ignore_monsters))
				continue;

			pResult = pNode;
			min_distance = distance;
		}
	}

	return min_distance;
}

std::shared_ptr<Node> NodeManager::GetAimNode(edict_t* pEntity, float width, float max_radius, std::shared_ptr<Node> pIgnoreNode) const
{
	Vector start = pEntity->v.origin + pEntity->v.view_ofs;
	Vector end;

	MAKE_VECTORS(pEntity->v.v_angle);
	end = start + (gpGlobals->v_forward * 4096);

	std::shared_ptr<Node> pAimNode = nullptr;
	float min_distance = width;

	for (auto &pCurrentNode : m_nodes)
	{
		if (pCurrentNode == pIgnoreNode)
			continue;

		Vector pos = pCurrentNode->GetPosition();

		if ((pos - pEntity->v.origin).Length() > max_radius)
			continue;

		if (!UTIL_IsVisible(pEntity->v.origin, pos, pEntity, ignore_monsters))
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

bool NodeManager::SaveFile(const char* pszFile)
{
	FILE* pFile = fopen(pszFile, "w");
	if (pFile)
	{
		fputs("[node]\n", pFile);

		for (auto it = m_nodes.begin(); it != m_nodes.end(); it++)
		{
			std::shared_ptr<Node> pNode = *it;
			Vector pos = pNode->GetPosition();

			fprintf(pFile, "%f %f %f %d\n", pos.x, pos.y, pos.z, pNode->GetFlags());
		}

		fputs("[path]\n", pFile);

		for (auto it = m_paths.begin(); it != m_paths.end(); it++)
		{
			std::shared_ptr<Path> pPath = *it;
			std::shared_ptr<Node> pBegin = pPath->GetBegin();
			std::shared_ptr<Node> pEnd = pPath->GetEnd();

			fprintf(pFile, "%d %d %d\n", NODE_INDEX(pBegin), NODE_INDEX(pEnd), pPath->GetFlags());
		}

		fclose(pFile);

		return true;
	}

	return false;
}

bool NodeManager::LoadFile(const char* pszFile)
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

				std::shared_ptr<Node> pBegin = GetNodeAt(begin_index);
				std::shared_ptr<Node> pEnd = GetNodeAt(end_index);

				if (pBegin != nullptr && pEnd != nullptr)
					this->CreatePath(pBegin, pEnd, flags);
			}
		}

		fclose(pFile);

		return true;
	}

	return false;
}