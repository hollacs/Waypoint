#pragma once

#ifdef WAYPOINT_EXPORTS
#  define WAYPOINT_API __declspec(dllexport)
#else
#  define WAYPOINT_API __declspec(dllimport)
#endif

#include <vector>

#include "extdll.h"

#define FL_NODE_JUMP (1 << 0)
#define FL_NODE_DUCK (1 << 1)

class Path;

class Node
{
public:
	Node(const Vector& origin, int flags);

	WAYPOINT_API Vector GetPosition() const;
	WAYPOINT_API int GetFlags() const;

	void SetPosition(const Vector& pos);
	WAYPOINT_API void SetFlags(int flags);

	WAYPOINT_API Path *GetPath(Node* pNode) const;
	void AddPath(Path* pPath);
	void RemovePath(Path* pPath);

	const std::vector<Path*>& GetPaths() const;
	std::vector<Path*>& GetPaths();

private:
	std::vector<Path*> m_paths;
	Vector m_position;
	int m_flags;
};