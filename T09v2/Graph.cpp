#include<iostream>
#include<fstream>
#include "Graph.h"
#include "Node.h"

using namespace std;

Graph* Graph::m_instance;

Graph::Graph()  {}

Graph* Graph::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new Graph();
		m_instance->m_currentIndex = -1;
	}
	return m_instance;
}

int Graph::GetIntersectionIndex(const pair<int, int>& intersection)
{
	return m_instance->m_streetsToIndex[intersection];
}

int Graph::GetCurrentIndex()
{
	return m_currentIndex;
}

void Graph::SetCurrentIndex(int ind)
{
	m_currentIndex = ind;
}

pair<int, int> Graph::GetMapKey(int street1, int street2)
{
	// the first street is the even one (i.e. W-E street)
	// the second street is the odd one (i.e. N-S street)
	if (street1 % 2 == 0) 
		return make_pair(street1, street2);
	return make_pair(street2, street1);
}

bool Graph::ExistsNodeInGraph(const std::pair<int, int>& intersection)
{
	return m_streetsToIndex.find(intersection) != m_streetsToIndex.end();
}

void Graph::AddEdgeAndNodes(int streetNumber, int crossStreet1, int crossStreet2, int streetType)
{
	int currentIndex = m_instance->GetCurrentIndex();
	pair<int, int> firstIntersection = GetMapKey(streetNumber, crossStreet1);
	pair<int, int> secondIntersection = GetMapKey(streetNumber, crossStreet2);
	int firstIntersectionIndex, secondIntersectionIndex;
	if (ExistsNodeInGraph(firstIntersection))
	{
		firstIntersectionIndex = m_instance->GetIntersectionIndex(firstIntersection);
	}
	else
	{
		firstIntersectionIndex = ++currentIndex;
	    Node *aux = new Node(firstIntersection.first, firstIntersection.second, firstIntersectionIndex);
		m_nodes.push_back(aux);
	}

	if (ExistsNodeInGraph(secondIntersection))
	{
		secondIntersectionIndex = m_instance->GetIntersectionIndex(secondIntersection);
	}
	else
	{
		secondIntersectionIndex = ++currentIndex;
		Node *aux = new Node(secondIntersection.first, secondIntersection.second, secondIntersectionIndex);
		m_nodes.push_back(aux);
	}

	//complete the list of edges
	m_nodes[firstIntersectionIndex]->AddEdge(m_nodes[secondIntersectionIndex]);
	if (streetType == 2)
	{
		m_nodes[secondIntersectionIndex]->AddEdge(m_nodes[firstIntersectionIndex]);
	}

	m_instance->SetCurrentIndex(currentIndex);
}

int Graph::GetNodesCount() const
{
	return m_nodes.size();
}

void Graph::Reset()
{
	// TODO
}

const std::vector<Node*>& Graph::GetNodes()
{
	return m_nodes;
}

void Graph::FindAllPaths(Node *start, Node *end) 
{

}