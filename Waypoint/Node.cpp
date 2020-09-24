#include <algorithm>
#include "Node.h"
#include "Path.h"

Node::Node(const Vector& origin, int flags)
{
	m_position = origin;
	m_flags = flags;
}

Path *Node::GetPath(Node* pNode) const
{
	auto it = std::find_if(m_paths.begin(), m_paths.end(), 
		[pNode](const Path* pPath) { return pPath->GetEnd() == pNode; });

	if (it == m_paths.end())
		return nullptr;
	
	return *it;
}

void Node::RemovePath(Path* pPath)
{
	m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), pPath), m_paths.end());
}

Vector Node::GetPosition() const
{
	return m_position;
}

int Node::GetFlags() const
{
	return m_flags;
}

void Node::SetPosition(const Vector& pos)
{
	m_position = pos;
}

void Node::SetFlags(int flags)
{
	m_flags = flags;
}

void Node::AddPath(Path* pPath)
{
	m_paths.push_back(pPath);
}

const std::vector<Path*>& Node::GetPaths() const
{
	return m_paths;
}

std::vector<Path*>& Node::GetPaths()
{
	return m_paths;
}