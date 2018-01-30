#pragma once
#include <vector>

class Node
{
public:
	Node(int WEStreet, int NSStreet, int index);
	
	void SetIndex(int newIndex);
	int GetIndex();

	int GetWEStreet();
	int GetNSStreet();

	void AddEdge(Node *intersection);
	bool ExistsEdge(int index);

	static void FixIntersectionStreets(std::pair<int, int>& intersection);

private:
	int m_index;
	int m_WEStreet, m_NSStreet;
	std::vector<Node*> m_edges;
};
