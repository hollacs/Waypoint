#pragma once

#ifdef WAYPOINT_EXPORTS
#  define WAYPOINT_API __declspec(dllexport)
#else
#  define WAYPOINT_API __declspec(dllimport)
#endif

#include "Node.h"

#define FL_PATH_JUMP (1 << 0)

class Path
{
public:
	Path(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, int flags);

	WAYPOINT_API std::shared_ptr<Node> GetBegin() const;
	WAYPOINT_API std::shared_ptr<Node> GetEnd() const;
	WAYPOINT_API int GetFlags() const;

	void SetBegin(std::shared_ptr<Node> pNode);
	void SetEnd(std::shared_ptr<Node> pNode);
	void SetFlags(int flags);

private:
	std::shared_ptr<Node> m_pBegin;
	std::shared_ptr<Node> m_pEnd;
	int m_flags;
};