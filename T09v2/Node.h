#pragma once
#include <vector>

class Node
{
public:
	Node(int WEStreet, int NSStreet, int index);
	
	void SetIndex(int newIndex);
	int GetIndex();

	int GetWEStreet() const;
	int GetNSStreet() const;

	void AddEdge(Node *intersection);
	void ShowNode();
	const std::vector<Node*>& GetEdges();

	static int GetBlocksBetween(const Node& A, const Node& B);

private:
	int m_index;
	int m_WEStreet, m_NSStreet;
	std::vector<Node*> m_edges;
};
