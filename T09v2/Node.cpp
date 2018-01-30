#include "Node.h"
#include <vector>

using namespace std;

Node::Node(int WEStreet, int NSStreet, int index) :
	m_WEStreet(WEStreet),
	m_NSStreet(NSStreet),
	m_index(index)
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

bool Node::ExistsEdge(int index)
{
	for (auto it = m_edges.begin(); it != m_edges.end(); ++it)
		if ((*it)->m_index == index)
			return true;
	return false;
}

void Node::FixIntersectionStreets(pair<int, int>& intersection)
{
	// in a intersection, the first street is the WE and the second is the NS
	// even == WE, odd = NS
	if (intersection.first % 2 == 1)
	{
		swap(intersection.first, intersection.second);
	}
}

void Node::AddEdge(Node *intersection)
{
	if (!ExistsEdge(intersection->m_index))
		m_edges.push_back(intersection);
}