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

private:
	int m_index;
	int m_WEStreet, m_NSStreet;
	std::vector<Node*> m_edges;
};
