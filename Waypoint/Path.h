#pragma once

#ifdef WAYPOINT_EXPORTS
#  define WAYPOINT_API __declspec(dllexport)
#else
#  define WAYPOINT_API __declspec(dllimport)
#endif

#include <memory>

#include "Node.h"

#define FL_PATH_JUMP (1 << 0)

class Path
{
public:
	Path(Node* pBegin, Node* pEnd, int flags);

	WAYPOINT_API Node* GetBegin() const;
	WAYPOINT_API Node* GetEnd() const;
	WAYPOINT_API int GetFlags() const;

	void SetBegin(Node* pNode);
	void SetEnd(Node* pNode);
	void SetFlags(int flags);

private:
	Node* m_pBegin;
	Node* m_pEnd;
	int m_flags;
};