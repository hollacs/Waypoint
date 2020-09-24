#pragma once

#include <vector>

#include "extdll.h"

class Node;
class Path;

class NodeManager
{
public:
	Node* CreateNode(const Vector& origin, int flags);
	void RemoveNode(Node* pNode);
	Node* GetNodeAt(size_t index) const;
	int GetNodeIndex(Node* pNode) const;
	size_t GetNodeSize() const { return m_nodes.size(); }
	const std::vector<Node*>& GetNodes() const { return m_nodes; }
	std::vector<Node*>& GetNodes() { return m_nodes; }
	Node* GetClosestNode(const Vector& origin, float radius, bool visible=false) const;
	Node* GetClosestNodeInView(const Vector& origin, const Vector& angle, float radius, float fov=90.0f, bool visible = false, bool use_2d = false) const;
	Node* GetAimNode(edict_t* pEntity, float radius, float max_radius, Node *pIgnoreNode=nullptr) const;

	Path* CreatePath(Node* pBegin, Node* pEnd, int flags);
	void RemovePath(Path* pPath);
	Path* GetPathAt(size_t index) const;
	int GetPathIndex(Path* pPath) const;
	const std::vector<Path*>& GetPaths() const { return m_paths; }
	std::vector<Path*>& GetPaths() { return m_paths; }
	size_t GetPathSize() const { return m_paths.size(); }

	void Clear();

	void SaveFile(const char* pszFile);
	void LoadFile(const char* pszFile);

private:
	std::vector<Node*> m_nodes;
	std::vector<Path*> m_paths;
};

