#pragma once
#include <vector>

class Node
{
public:
	Node(int WEStreet, int NSStreet);
	
	void SetIndex(int newIndex);
	int GetIndex();

	int GetWEStreet();
	int GetNSStreet();

	void AddEdge(Node *intersection);
	bool isExistingEdge(int index);

private:
	int m_index;
	int m_WEStreet, m_NSStreet;
	std::vector<Node*> m_edges;
};
