#pragma once

class INode;

class IPath
{
public:
	virtual ~IPath() {};

	virtual INode* GetBegin() const = 0;
	virtual INode* GetEnd() const = 0;
	virtual int GetFlags() const = 0;
};