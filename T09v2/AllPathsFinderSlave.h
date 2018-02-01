#pragma once
#include "IAllPathsFinder.h"
#include <vector>

class Graph;
class GraphPath;

class AllPathsFinderSlave : public IAllPathsFinder
{
public:
	AllPathsFinderSlave();
	virtual void FindAllPaths(int startingNodeIndex, int destinationNodeIndex);
	virtual const std::vector<GraphPath*>& GetAllPaths();
	virtual ~AllPathsFinderSlave();
private:
	void FindAllPathsToDestination(int startingPathLength, int* startingPath);
	void DFSFindAllPathsToDestination(int startingNode, int stackLevel);
	void SendAllPathsToMaster();
	int m_startingNodeIndex, m_destinationNodeIndex;
	std::vector<int> m_allFoundPaths;

	std::vector<GraphPath*> m_dummyAllPaths;
	Graph* m_graph;
	int *m_auxPathStack = nullptr;
	bool *m_viz = nullptr;
};