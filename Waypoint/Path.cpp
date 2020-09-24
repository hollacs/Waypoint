#include "Path.h"

Path::Path(Node* pBegin, Node* pEnd, int flags)
{
	m_pBegin = pBegin;
	m_pEnd = pEnd;
	m_flags = flags;
}

Node* Path::GetBegin() const
{
	return m_pBegin;
}

Node* Path::GetEnd() const
{
	return m_pEnd;
}

int Path::GetFlags() const
{
	return m_flags;
}

void Path::SetBegin(Node* pNode)
{
	m_pBegin = pNode;
}

void Path::SetEnd(Node* pNode)
{
	m_pEnd = pNode;
}

void Path::SetFlags(int flags)
{
	m_flags = flags;
}