#include "Node.h"
#include <vector>

using namespace std;

Node::Node(int WEStreet, int NSStreet, int index) :
	m_WEStreet(WEStreet),
	m_NSStreet(NSStreet),
	m_index(index)
{ }

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

void Node::AddEdge(Node *intersection)
{
	m_edges.push_back(intersection);
}