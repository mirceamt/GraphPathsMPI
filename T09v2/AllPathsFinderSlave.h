#pragma once
#include "IAllPathsFinder.h"

class GraphPath;

class AllPathsFinderSlave : public IAllPathsFinder
{
public:
	AllPathsFinderSlave();
	virtual void FindAllPaths(int startingNodeIndex, int destinationNodeIndex);
	virtual const std::vector<GraphPath*>& GetAllPaths();
private:
	
};