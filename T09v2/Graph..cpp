#include<iostream>
#include<fstream>
#include "Graph.h"
#include "Node.h"

Graph::Graph()  {}

Graph* Graph::GetInstance()
{
	if (m_Instance == NULL)
	{
		m_Instance = new Graph();
		m_Instance->m_currentIndex = -1;
	}
	return m_Instance;
}

int Graph::GetInsersectionIndex(std::pair<int, int> intersection)
{
	return m_Instance->m_StreetsToIndex[intersection];
}

int Graph::GetCurrentIndex()
{
	return m_currentIndex;
}

void Graph::SetCurrentIndex(int ind)
{
	m_currentIndex = ind;
}

std::pair<int, int> Graph::GetMapKey(int street1, int street2)
{
	if (street1 % 2 == 0) return std::make_pair(street1, street2);
	return std::make_pair(street2, street1);
}


bool Graph::ExistsInGraph(std::pair<int, int> intersection)
{
	return m_Instance->m_StreetsToIndex.find(intersection) != m_Instance->m_StreetsToIndex.end();
}

void Graph::AddNode(int streetNumber, int crossStreet1, int crossStreet2, int streetType)
{

	int currentIndex = m_Instance->GetCurrentIndex;
	std::pair<int, int> firstIntersection = GetMapKey(streetNumber, crossStreet1);
	std::pair<int, int> secondIntersection = GetMapKey(streetNumber, crossStreet2);
	int firstIntersectionIndex, secondIntersectionIndex;
	if (ExistsInGraph(firstIntersection))
	{
		firstIntersectionIndex = m_Instance->GetInsersectionIndex(firstIntersection);
	}
	else
	{
		firstIntersectionIndex = ++currentIndex;
	    Node *aux = new Node(firstIntersection.first, firstIntersection.second);
		aux->SetIndex(firstIntersectionIndex);
		m_Instance->m_Nodes.push_back(aux);
	}

	if (ExistsInGraph(secondIntersection))
	{
		secondIntersectionIndex = m_Instance->GetInsersectionIndex(secondIntersection);
	}
	else
	{
		secondIntersectionIndex = ++currentIndex;
		Node *aux = new Node(secondIntersection.first, secondIntersection.second);
		aux->SetIndex(secondIntersectionIndex);
		m_Instance->m_Nodes.push_back(aux);
	}

	//complete the list of edges
	m_Instance->m_Nodes[firstIntersectionIndex]->AddEdge(m_Instance->m_Nodes[secondIntersectionIndex]);
	if(streetType == 2)
		m_Instance->m_Nodes[secondIntersectionIndex]->AddEdge(m_Instance->m_Nodes[firstIntersectionIndex]);


	m_Instance->SetCurrentIndex(currentIndex);

}

void FindAllPaths(Node *start, Node *end) 
{


}