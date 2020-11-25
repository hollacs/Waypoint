#include "Node.h"

#include <algorithm>

#include "Path.h"

Node::Node(const Vector& origin, int flags)
{
	m_position = origin;
	m_flags = flags;
}

std::shared_ptr<Path> Node::GetPath(std::shared_ptr<Node> pNode) const
{
	auto iter = std::find_if(m_paths.begin(), m_paths.end(),
		[pNode](const std::shared_ptr<Path>& pPath) {
			return pPath->GetEnd() == pNode;
		});

	if (iter == m_paths.end())
		return nullptr;

	return *iter;
}

void Node::RemovePath(std::shared_ptr<Path> pRemovePath)
{
	if (pRemovePath == nullptr)
		return;

	m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), pRemovePath), m_paths.end());
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

void Node::AddPath(std::shared_ptr<Path> pPath)
{
	if (pPath == nullptr)
		return;

	m_paths.push_back(pPath);
}

const std::vector<std::shared_ptr<Path>>& Node::GetPaths() const
{
	return m_paths;
}

std::vector<std::shared_ptr<Path>>& Node::GetPaths()
{
	return m_paths;
}