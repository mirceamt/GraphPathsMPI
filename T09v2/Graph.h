#pragma once
#include<map>
#include<vector>
class Node;

class Graph
{
public:
	static Graph* GetInstance();
	void AddNode(int streetNumber, int crossStreet1, int crossStreet2, int streetType);
	std::vector<Node> GetNodes();
	std::pair<int, int> GetMapKey(int street1, int street2);
	int GetInsersectionIndex(std::pair<int, int> intersection);
	int GetCurrentIndex();
	void SetCurrentIndex(int ind);
	void FindAllPaths(Node* start, Node* end);
	bool ExistsInGraph(std::pair<int, int> intersection);
private:
	Graph();
	int m_currentIndex;
	static Graph* m_Instance;
	std::map<std::pair<int, int>, int> m_StreetsToIndex;
	std::vector<Node *> m_Nodes;
};