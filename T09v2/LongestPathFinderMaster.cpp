#include "LongestPathFinderMaster.h"
#include "Graph.h"
#include "Node.h"
#include "GraphPath.h"
#include "Master.h"
#include "CommonUtils.h"
#include "SlavePool.h"
#include "mpi.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>

using namespace std;

LongestPathFinderMaster::LongestPathFinderMaster()
{
	m_graph = Graph::GetInstance();
	int nodesCount = m_graph->GetNodesCount();

	m_viz = new bool[nodesCount];
	m_auxPathStack = new int[nodesCount];
	for (int i = 0; i < nodesCount; ++i)
	{
		m_viz[i] = false;
		m_auxPathStack[i] = 0;
	}
}

void LongestPathFinderMaster::FindLongestPath(int startingNodeIndex, int destinationNodeIndex)
{
	m_startingNodeIndex = startingNodeIndex;
	m_destinationNodeIndex = destinationNodeIndex;

	const int totalProcesses = CommonUtils::GetNrProcesses();
	int nrSlaves = totalProcesses - 1;
	FindAllStartingPaths(startingNodeIndex, nrSlaves); // the result is found in m_allStartingPaths

	ShowAllStartingPaths();

	int allStartingPathsIndex = 0;
	SlavePool *slavePool = new SlavePool();

	for (int i = 1; i <= min(nrSlaves, (int)m_allStartingPaths.size()); ++i)
	{
		vector<int>& startingPath = m_allStartingPaths[allStartingPathsIndex];
		allStartingPathsIndex++;
		slavePool->DoJob(startingPath);
	}
	slavePool->StopIdleSlaves(); // in case there are more slaves than jobs; The remaining idle slaves need to be stopped.

								 /*Master::Log("NUMBER OF SLAVES: " + CommonUtils::IntToString(nrSlaves));
								 Master::Log("stopped slaves: " + CommonUtils::IntToString(slavePool->GetStoppedSlavesCount()));*/
	while (slavePool->GetStoppedSlavesCount() != nrSlaves)
	{
		//Master::Log("NUMBER OF SLAVES: " + CommonUtils::IntToString(nrSlaves));
		//Master::Log("stopped slaves: " + CommonUtils::IntToString(slavePool->GetStoppedSlavesCount()));

		int firstMessage[2]; 
		// can be 0 0 - meaning that the slave found no paths to destination.
		// x y - meaning that the longest path found to destination has y blockLength and consists of x intersections.

		int& messageLength = firstMessage[0];
		int& blockLength = firstMessage[1];

		MPI_Status status;
		MPI_Recv(firstMessage, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		int source = status.MPI_SOURCE;
		Master::Log("Received message from " + CommonUtils::IntToString(source) + " message: " + CommonUtils::IntsToString(firstMessage, 2));
		int *message = nullptr;
		if (messageLength != 0) // messageLength is 0 when the slave found no paths to destination
		{
			message = new int[messageLength];
			MPI_Recv(message, messageLength, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			Master::Log("Received message from " + CommonUtils::IntToString(source) + " message: " + CommonUtils::IntsToString(message, messageLength));
		}

		//slavePool->ShowVectors();
		slavePool->ChangeSlaveStatus(source, SlaveStatus::Working, SlaveStatus::Idle);
		//slavePool->ShowVectors();
		//cout.flush();

		if (allStartingPathsIndex < (int)m_allStartingPaths.size())
		{
			vector<int>& startingPath = m_allStartingPaths[allStartingPathsIndex];
			allStartingPathsIndex++;
			slavePool->DoJob(startingPath);
		}
		else
		{
			Master::Log("Stopping slaves because of lack of jobs");
			slavePool->StopIdleSlaves();
		}

		if (messageLength != 0) // process the paths in case the slave generated some paths.
		{
			//Master::Log("11111111111111111111111111111111111111111111");
			ProcessReceivedPath(messageLength, message, blockLength);
			//Master::Log("22222222222222222222222222222222222222222222");
			delete[] message;
		}
	}
	delete slavePool;
}

void LongestPathFinderMaster::ProcessReceivedPath(int msgLength, int* msg, int blocksLength)
{
	if (m_longestPath == nullptr || m_longestPath->GetBlocksLength() < blocksLength)
	{
		if (m_longestPath != nullptr)
		{
			delete m_longestPath;
		}
		m_longestPath = new GraphPath(msgLength, msg);
	}
}

void LongestPathFinderMaster::ShowLongestPath()
{
	cout << "All paths from ";
	m_graph->GetNodes()[m_startingNodeIndex]->ShowNode();
	cout << " to ";
	m_graph->GetNodes()[m_destinationNodeIndex]->ShowNode();
	cout << "\n";
	cout.flush();

	if (m_longestPath == nullptr)
	{
		cout << "\nNo path found!\n";
		cout.flush();
		return;
	}
	cout << "\n";
	cout.flush();
	m_longestPath->ShowNodes();
	cout << "\n";
	cout.flush();
}

void LongestPathFinderMaster::ShowAllStartingPaths()
{
	cout << "\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n";
	cout.flush();
	for (auto& x : m_allStartingPaths)
	{
		for (auto y : x)
		{
			cout << y << " ";
			cout.flush();
		}
		cout << "\n";
		cout.flush();
	}
	cout << "\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n";
	cout.flush();
}

GraphPath* LongestPathFinderMaster::GetLongestPath()
{
	return m_longestPath;
}

void LongestPathFinderMaster::DFSCountPaths(int nodeIndex, int d, const int maxDistance)
{
	if (m_viz[nodeIndex])
	{
		return;
	}

	if (d == maxDistance)
	{
		++m_auxPathsCount;
		return;
	}
	m_viz[nodeIndex] = true;

	Node* node = m_graph->GetNodes()[nodeIndex];
	const vector<Node*>& edges = node->GetEdges();
	bool couldContinue = false;
	for (vector<Node*>::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		if (!m_viz[(*it)->GetIndex()])
		{
			couldContinue = true;
			DFSCountPaths((*it)->GetIndex(), d + 1, maxDistance);
		}
	}

	if (couldContinue == false)
	{
		++m_auxShorterPaths;
	}

	m_viz[nodeIndex] = false;
}

void LongestPathFinderMaster::ComputePathsCount(int startingNodeIndex, int maxDistance)
{
	m_auxPathsCount = 0;
	m_auxShorterPaths = 0;
	DFSCountPaths(startingNodeIndex, 0, maxDistance);
}

void LongestPathFinderMaster::DFSGetStartingPaths(int nodeIndex, int d, const int maxDistance, int blocksLength)
{
	if (m_viz[nodeIndex])
	{
		return;
	}
	m_auxPathStack[d] = nodeIndex;
	if (nodeIndex == m_destinationNodeIndex)
	{
		// found a path to destination, shorter than maxDistance
		ProcessReceivedPath(d + 1, m_auxPathStack, blocksLength);
		return;
	}

	if (d == maxDistance)
	{
		// create a startingPath from m_auxPathStack;
		vector<int> startingPath;
		for (int i = 0; i <= d; ++i)
		{
			startingPath.push_back(m_auxPathStack[i]);
		}
		m_allStartingPaths.push_back(startingPath);
		return;
	}
	m_viz[nodeIndex] = true;

	Node* node = m_graph->GetNodes()[nodeIndex];
	const vector<Node*>& edges = node->GetEdges();
	for (vector<Node*>::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		int blocksBetween = Node::GetBlocksBetween(*node, *(*it));
		DFSGetStartingPaths((*it)->GetIndex(), d + 1, maxDistance, blocksLength + blocksBetween);
	}

	m_viz[nodeIndex] = false;
}

void LongestPathFinderMaster::PopulateAllStartingPaths(int startingNodeIndex, int bestDistance)
{
	DFSGetStartingPaths(startingNodeIndex, 0, bestDistance, 0);
}

void LongestPathFinderMaster::FindAllStartingPaths(int startingNodeIndex, int nrSlaves)
{
	// Define a path as a sequence of non-repeating adjacent nodes.
	// Define the length of a path as the number of nodes in that path.
	// This method finds the lowest X which ensures that the number of paths of length X, 
	// starting from the startingNode, is >= nrSlaves using binary search and returns
	// those paths in m_allStartingPaths

	int step = 1;
	int currentDistance = 1, bestDistance = 1;
	int oldPathsCount = -1;
	while (true)
	{
		ComputePathsCount(startingNodeIndex, currentDistance);
		int pathsCount = m_auxPathsCount + m_auxShorterPaths;
		if (oldPathsCount == pathsCount)
		{
			step = step >> 1;
			currentDistance -= step;
			bestDistance = currentDistance;
			break;
		}
		oldPathsCount = pathsCount;

		if (pathsCount < nrSlaves)
		{
			currentDistance += step;
			step = step << 1;
		}
		else
		{
			bestDistance = currentDistance;
			break;
		}
	}
	step = step >> 1;
	currentDistance -= step;
	while (step > 0)
	{
		ComputePathsCount(startingNodeIndex, currentDistance);
		int pathsCount = m_auxPathsCount + m_auxShorterPaths;
		if (pathsCount < nrSlaves)
		{
			currentDistance += step;
		}
		else
		{
			bestDistance = currentDistance;
			currentDistance -= step;
		}

		step = step >> 1;
	}

	PopulateAllStartingPaths(startingNodeIndex, bestDistance);
}

LongestPathFinderMaster::~LongestPathFinderMaster()
{
	delete[] m_viz;
	delete[] m_auxPathStack;
	if (m_longestPath != nullptr)
	{
		delete m_longestPath;
		m_longestPath = nullptr;
	}
}


