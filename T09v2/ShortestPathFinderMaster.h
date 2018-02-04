#pragma once
#include "IShortestPathFinder.h"

class Graph;
class GraphPath;

class ShortestPathFinderMaster : public IShortestPathFinder
{
public:
	ShortestPathFinderMaster(int *nodesIntervals, int nodesIntervalsLength, int rank);
	virtual void FindShortestPath(int startingNodeIndex, int destinationNodeIndex);
	void ShowShortestPath();
	virtual ~ShortestPathFinderMaster();
private:
	void FindLocalMin();
	int LocalToGlobal(int nodeIndex);
	int GlobalToLocal(int nodeIndex);
	bool IsGlobalNodeHere(int globalNodeIndex);

	int m_startingNodeIndex, m_destinationNodeIndex;
	int m_localMin, m_localMinPos;
	int m_globalLowNode, m_globalHighNode;
	int m_localLowNode, m_localHighNode, m_localNodesCount;
	Graph* m_graph;
	GraphPath* m_shortestPath = nullptr;
	int* m_nodesIntervals;
	int m_nodesIntervalsLength;
	bool* m_known = nullptr;
	int* m_dist = nullptr;
	int* m_pred = nullptr;
	int m_rank;
};