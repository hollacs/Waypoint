#include "Path.h"

Path::Path(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, int flags)
{
	m_pBegin = pBegin;
	m_pEnd = pEnd;
	m_flags = flags;
}

std::shared_ptr<Node> Path::GetBegin() const
{
	return m_pBegin;
}

std::shared_ptr<Node> Path::GetEnd() const
{
	return m_pEnd;
}

int Path::GetFlags() const
{
	return m_flags;
}

void Path::SetBegin(std::shared_ptr<Node> pNode)
{
	m_pBegin = pNode;
}

void Path::SetEnd(std::shared_ptr<Node> pNode)
{
	m_pEnd = pNode;
}

void Path::SetFlags(int flags)
{
	m_flags = flags;
}