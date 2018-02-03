#pragma once
#include "ILongestPathFinder.h"
#include <vector>

class Graph;
class GraphPath;

class LongestPathFinderSlave : public ILongestPathFinder
{
public:
	LongestPathFinderSlave();
	virtual void FindLongestPath(int startingNodeIndex, int destinationNodeIndex);
	virtual GraphPath* GetLongestPath();
	virtual ~LongestPathFinderSlave();
private:
	void FindLongestPathToDestination(int startingPathLength, int* startingPath);
	void DFSFindLongestPathToDestination(int startingNode, int stackLevel, int startingPathBlocksLength);
	void SendLongestPathToMaster();
	int m_startingNodeIndex, m_destinationNodeIndex;

	Graph* m_graph;
	GraphPath* m_longestPath = nullptr;
	int *m_auxPathStack = nullptr;
	bool *m_viz = nullptr;
};