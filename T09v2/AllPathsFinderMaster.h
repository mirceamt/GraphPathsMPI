#pragma once
#include "IAllPathsFinder.h"
#include <vector>

class Graph;
class GraphPath;

class AllPathsFinderMaster : public IAllPathsFinder
{
public:
	AllPathsFinderMaster();
	virtual void FindAllPaths(int startingNodeIndex, int destionationNodeIndex);
	virtual const std::vector<GraphPath*>& GetAllPaths();
	virtual ~AllPathsFinderMaster();
	void ShowAllPaths();
private:
	void ShowAllStartingPaths();
	
	void DFSCountPaths(int startingNodeIndex, int d, const int maxDistance);
	void ComputePathsCount(int startingNodeIndex, int currentDistance);

	void DFSGetStartingPaths(int startingNodeIndex, int d, const int maxDistance);
	void PopulateAllStartingPaths(int startingNodeIndex, int bestDistance);

	void FindAllStartingPaths(int startingNodeIndex, int nrSlaves);

	void ProcessReceivedPaths(int msgLength, int* msg);

	int m_startingNodeIndex;
	int m_destinationNodeIndex;
	bool *m_viz;
	int m_auxPathsCount;
	int m_auxShorterPaths;
	int *m_auxPathStack;
	Graph* m_graph;
	std::vector<GraphPath*> m_allPaths;
	std::vector<std::vector<int>> m_allStartingPaths;
};