#include "Node.h"
#include <vector>

Node::Node(int WEStreet, int NSStreet) :
	m_WEStreet(WEStreet),
	m_NSStreet(NSStreet)
{
}

void Node::SetIndex(int newIndex)
{
	m_index = newIndex;
}

int Node::GetIndex()
{
	return m_index;
}


int Node::GetWEStreet()
{
	return m_WEStreet;
}

int Node::GetNSStreet()
{
	return m_NSStreet;
}

bool Node::isExistingEdge(int index)
{
	for (auto it = m_edges.begin(); it != m_edges.end(); ++it)
		if ((*it)->m_index == index)
			return true;
	return false;

}

void Node::AddEdge(Node *intersection)
{
	if (!isExistingEdge(intersection->m_index))
		m_edges.push_back(intersection);
}