#include "AllPathsFinderMaster.h"
#include "Graph.h"
#include "Node.h"
#include "GraphPath.h"
#include "CommonUtils.h"
#include "SlavePool.h"
#include "mpi.h"
#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

AllPathsFinderMaster::AllPathsFinderMaster()
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

void AllPathsFinderMaster::FindAllPaths(int startingNodeIndex, int destinationNodeIndex)
{
	m_startingNodeIndex = startingNodeIndex;
	m_destinationNodeIndex = destinationNodeIndex;

	const int totalProcesses = CommonUtils::GetNrProcesses();
	int nrSlaves = totalProcesses - 1;
	FindAllStartingPaths(startingNodeIndex, nrSlaves); // the result is found in m_allStartingPaths

	int allStartingPathsIndex = 0;
	SlavePool *slavePool = new SlavePool();

	for (int i = 1; i <= nrSlaves; ++i)
	{
		vector<int>& startingPath = m_allStartingPaths[allStartingPathsIndex];
		allStartingPathsIndex++;
		slavePool->DoJob(startingPath);
	}

	while (slavePool->GetStoppedSlavesCount() != nrSlaves)
	{
		int messageLength;
		MPI_Status status;
		MPI_Recv(&messageLength, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		int source = status.MPI_SOURCE;
		int *message = new int[messageLength];
		MPI_Recv(message, messageLength, MPI_INT, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		
		slavePool->ChangeSlaveStatus(source, SlaveStatus::Working, SlaveStatus::Idle);

		if (allStartingPathsIndex < (int)m_allStartingPaths.size())
		{
			vector<int>& startingPath = m_allStartingPaths[allStartingPathsIndex];
			allStartingPathsIndex++;
			slavePool->DoJob(startingPath);
		}
		else
		{
			slavePool->StopIdleSlaves();
		}
		ProcessReceivedPaths(messageLength, message);
		delete[] message;
	}

	delete slavePool;
}

void AllPathsFinderMaster::ProcessReceivedPaths(int msgLength, int* msg)
{
	int i = 0;
	while (i < msgLength)
	{
		GraphPath* graphPath = new GraphPath(msg[i], msg + i + 1);
		m_allPaths.push_back(graphPath);
		i = i + msg[i] + 1;
	}
}

void AllPathsFinderMaster::ShowAllPaths()
{
	cout << "All paths from ";
	m_graph->GetNodes()[m_startingNodeIndex]->ShowNode();
	cout << " to ";
	m_graph->GetNodes()[m_destinationNodeIndex]->ShowNode();
	cout << "\n";
	cout.flush();

	if (m_allPaths.empty())
	{
		cout << "No paths found!";
		cout.flush();
	}
	else
	{
		for (int i = 0; i < (int)m_allPaths.size(); ++i)
		{
			cout << "Path " << i + 1 << ": ";
			cout.flush();
			m_allPaths[i]->ShowNodes();
			cout << "\n";
			cout.flush();
		}
	}
}

const vector<GraphPath*>& AllPathsFinderMaster::GetAllPaths()
{
	return m_allPaths;
}

void AllPathsFinderMaster::DFSCountPaths(int nodeIndex, int d, const int maxDistance)
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

void AllPathsFinderMaster::ComputePathsCount(int startingNodeIndex, int maxDistance)
{
	m_auxPathsCount = 0;
	m_auxShorterPaths = 0;
	DFSCountPaths(startingNodeIndex, 0, maxDistance);
}

void AllPathsFinderMaster::DFSGetStartingPaths(int nodeIndex, int d, const int maxDistance)
{
	if (m_viz[nodeIndex])
	{
		return;
	}
	m_auxPathStack[d] = nodeIndex;
	if (nodeIndex == m_destinationNodeIndex)
	{
		// found a path to destination, shorter than maxDistance
		GraphPath* graphPath = new GraphPath(d, m_auxPathStack);
		m_allPaths.push_back(graphPath);
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
		DFSGetStartingPaths((*it)->GetIndex(), d + 1, maxDistance);
	}

	m_viz[nodeIndex] = false;
}

void AllPathsFinderMaster::PopulateAllStartingPaths(int startingNodeIndex, int bestDistance)
{
	DFSGetStartingPaths(startingNodeIndex, 0, bestDistance);
}

void AllPathsFinderMaster::FindAllStartingPaths(int startingNodeIndex, int nrSlaves)
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

AllPathsFinderMaster::~AllPathsFinderMaster()
{
	delete[] m_viz;
	delete[] m_auxPathStack;
	for (auto x : m_allPaths)
	{
		delete x;
	}
}


