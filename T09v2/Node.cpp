#include "Node.h"
#include<iostream>
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

int Node::GetWEStreet() const
{
	return m_WEStreet;
}

int Node::GetNSStreet() const
{
	return m_NSStreet;
}

void Node::AddEdge(Node *intersection)
{
	m_edges.push_back(intersection);
}

const std::vector<Node*>& Node::GetEdges()
{
	return m_edges;
}

int Node::GetBlocksBetween(const Node & A, const Node & B)
{
	return (abs(A.GetWEStreet() - B.GetWEStreet()) + abs(A.GetNSStreet() - B.GetNSStreet())) / 2;
}

void Node::ShowNode(bool flush)
{
	cout << "(" << m_WEStreet << ", " << m_NSStreet << ")";
	if (flush)
	{
		cout.flush();
	}
}