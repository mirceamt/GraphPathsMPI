#pragma once
#include <vector>

class GraphPath;

class ILongestPathFinder
{
public:
	virtual void FindLongestPath(int startingNodeIndex, int destinationNodeIndex) = 0;
	virtual GraphPath* GetLongestPath() = 0;
};