#pragma once

#ifdef WAYPOINT_EXPORTS
#  define WAYPOINT_API __declspec(dllexport)
#else
#  define WAYPOINT_API __declspec(dllimport)
#endif

#include <memory>
#include <vector>

#include "extdll.h"

#define WP_FL_JUMP (1 << 0)
#define WP_FL_DUCK (1 << 1)
#define WP_FL_LADDER (1 << 2)

class Path;

class Node
{
public:
	Node(const Vector& origin, int flags);

	WAYPOINT_API Vector GetPosition() const;
	WAYPOINT_API int GetFlags() const;

	void SetPosition(const Vector& pos);
	WAYPOINT_API void SetFlags(int flags);

	WAYPOINT_API std::shared_ptr<Path> GetPath(std::shared_ptr<Node> pNode) const;
	void AddPath(std::shared_ptr<Path> pPath);
	void RemovePath(std::shared_ptr<Path> pPath);

	const std::vector<std::shared_ptr<Path>>& GetPaths() const;
	std::vector<std::shared_ptr<Path>>& GetPaths();

private:
	std::vector<std::shared_ptr<Path>> m_paths;
	Vector m_position;
	int m_flags;
};