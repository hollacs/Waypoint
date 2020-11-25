#pragma once

#ifdef WAYPOINT_EXPORTS
#  define WAYPOINT_API __declspec(dllexport)
#else
#  define WAYPOINT_API __declspec(dllimport)
#endif

#include "extdll.h"

#include "Node.h"
#include "Path.h"

#define NODE_INDEX(n) GetNodeManager().GetNodeIndex(n)
#define PATH_INDEX(p) GetNodeManager().GetPathIndex(p)

class NodeManager
{
public:
	std::shared_ptr<Node> CreateNode(const Vector& origin, int flags);
	void RemoveNode(std::shared_ptr<Node> pNode);
	WAYPOINT_API std::shared_ptr<Node> GetNodeAt(size_t index);
	WAYPOINT_API int GetNodeIndex(std::shared_ptr<Node> pNode) const;
	WAYPOINT_API size_t GetNodeSize() const;
	const std::vector<std::shared_ptr<Node>>& GetNodes() const;
	std::vector<std::shared_ptr<Node>>& GetNodes();
	WAYPOINT_API float GetClosestNode(const Vector& origin, std::shared_ptr<Node>& pResult, float radius=4096.0f, bool visible=false) const;
	WAYPOINT_API std::shared_ptr<Node> GetAimNode(edict_t* pEntity, float radius, float max_radius, std::shared_ptr<Node> pIgnoreNode=nullptr) const;

	std::shared_ptr<Path> CreatePath(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, int flags);
	void RemovePath(std::shared_ptr<Path> pPath);
	WAYPOINT_API std::shared_ptr<Path> GetPathAt(size_t index) const;
	WAYPOINT_API int GetPathIndex(std::shared_ptr<Path> pPath) const;
	const std::vector<std::shared_ptr<Path>>& GetPaths() const;
	std::vector<std::shared_ptr<Path>>& GetPaths();
	WAYPOINT_API size_t GetPathSize() const;

	void Clear();

	bool SaveFile(const char* pszFile);
	bool LoadFile(const char* pszFile);

private:
	
	std::vector<std::shared_ptr<Node>> m_nodes;
	std::vector<std::shared_ptr<Path>> m_paths;
};

WAYPOINT_API NodeManager& GetNodeManager();