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