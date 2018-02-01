#include "GraphPath.h"
#include "Graph.h"
#include "Node.h"
#include <iostream>
#include <vector>

using namespace std;

GraphPath::GraphPath(int n, int* nodesIndices):
	m_count(n)
{
	m_v = new Node*[m_count];
	const vector<Node*>& graphNodes = (Graph::GetInstance())->GetNodes();
	for (int i = 0; i < m_count; ++i)
	{
		m_v[i] = graphNodes[nodesIndices[i]];
	}
}

GraphPath::GraphPath(const GraphPath& other)
{
	m_count = other.m_count;
	m_v = new Node*[m_count];
	for (int i = 0; i < m_count; ++i)
	{
		m_v[i] = other.m_v[i];
	}
}


int GraphPath::GetCount() const
{
	return m_count;
}

Node** GraphPath::GetNodes() const
{
	return m_v;
}

void GraphPath::ShowNodes()
{
	for (int i = 0; i < m_count; ++i)
	{
		m_v[i]->ShowNode();
		cout << " ";
		cout.flush();
	}
}

GraphPath::~GraphPath()
{
	delete[] m_v;
}
