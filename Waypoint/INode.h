#pragma once

#include <vector>

#include "extdll.h"

class IPath;

class INode
{
public:
	virtual ~INode() {};

	virtual Vector GetPosition() const = 0;
	virtual int GetFlags() const = 0;
	virtual IPath* GetPath(INode* pNode) const = 0;
	virtual const std::vector<IPath*>& GetPaths() const = 0;
	virtual void SetFlags(int flags) = 0;
};