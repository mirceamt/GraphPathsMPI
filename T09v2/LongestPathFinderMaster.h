#pragma once
#include "ILongestPathFinder.h"

class GraphPath;
class Graph;

class LongestPathFinderMaster : public ILongestPathFinder
{
public:
	LongestPathFinderMaster();
	virtual void FindLongestPath(int startingNodeIndex, int destinationNodeIndex);
	virtual GraphPath* GetLongestPath();
	~LongestPathFinderMaster();
	void ShowLongestPath();
private:
	void ShowAllStartingPaths();

	void DFSGetStartingPaths(int startingNodeIndex, int d, const int maxDistance, int blocksLength);
	void PopulateAllStartingPaths(int startingNodeIndex, int bestDistance);

	void DFSCountPaths(int startingNodeIndex, int d, const int maxDistance);
	void ComputePathsCount(int startingNodeIndex, int currentDistance);

	void FindAllStartingPaths(int startingNodeIndex, int nrSlaves);

	void ProcessReceivedPath(int msgLength, int* msg, int blockLength);


	int m_startingNodeIndex;
	int m_destinationNodeIndex;
	bool *m_viz;
	int m_auxPathsCount;
	int m_auxShorterPaths;
	int *m_auxPathStack;
	Graph* m_graph;
	GraphPath* m_longestPath = nullptr;
	std::vector<std::vector<int>> m_allStartingPaths;
};