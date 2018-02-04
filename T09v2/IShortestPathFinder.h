#pragma once

class IShortestPathFinder
{
public: 
	virtual void FindShortestPath(int startingNodeIndex, int destinationNodeIndex) = 0;
};