#pragma once
#include <vector>

class GraphPath;

class IAllPathsFinder
{
public:
	virtual void FindAllPaths(int startingNodeIndex, int destinationNodeIndex) = 0;
	virtual const std::vector<GraphPath*>& GetAllPaths() = 0;
};