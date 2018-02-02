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
	ComputeBlocksLength();
}

GraphPath::GraphPath(const GraphPath& other)
{
	m_count = other.m_count;
	m_v = new Node*[m_count];
	for (int i = 0; i < m_count; ++i)
	{
		m_v[i] = other.m_v[i];
	}
	ComputeBlocksLength();
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

int GraphPath::GetBlocksLength()
{
	return m_blocksLength;
}

void GraphPath::ComputeBlocksLength()
{
	m_blocksLength = 0;
	for (int i = 1; i < m_count; ++i)
	{
		Node* currentNode = m_v[i];
		Node* prevNode = m_v[i - 1];
		m_blocksLength += Node::GetBlocksBetween(*currentNode, *prevNode);
	}
}

GraphPath::~GraphPath()
{
	delete[] m_v;
}
