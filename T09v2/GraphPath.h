#pragma once

class Node;

class GraphPath
{
public:
	GraphPath(int n, int* nodesIndices);
	GraphPath(const GraphPath& other);
	int GetCount() const;
	Node** GetNodes() const;
	void ShowNodes();
	int GetBlocksLength();
	~GraphPath();

private:
	void ComputeBlocksLength();
	int m_blocksLength;
	int m_count;
	Node** m_v = nullptr;
};