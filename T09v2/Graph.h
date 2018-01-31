#pragma once
#include<map>
#include<vector>
class Node;

class Graph
{
public:
	static Graph* GetInstance();
	void AddEdgeAndNodes(int streetNumber, int crossStreet1, int crossStreet2, int streetType);
	const std::vector<Node*>& GetNodes();
	std::pair<int, int> GetMapKey(int street1, int street2);
	int GetIntersectionIndex(const std::pair<int, int>& intersection);
	void FindAllPaths(Node* start, Node* end);
	bool ExistsNodeInGraph(const std::pair<int, int>& intersection);
	void Reset();
private:
	Graph();
	int GetCurrentIndex();
	void SetCurrentIndex(int ind);

	int m_currentIndex;
	static Graph* m_instance;
	std::map<std::pair<int, int>, int> m_streetsToIndex;
	std::vector<Node*> m_nodes;
};