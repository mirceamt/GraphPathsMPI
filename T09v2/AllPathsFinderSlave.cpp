#include "AllPathsFinderSlave.h"
#include "Graph.h"
#include "GraphPath.h"
#include "Node.h"
#include "Slave.h"
#include <vector>
#include "mpi.h"
#include "CommonUtils.h"

using namespace std;

AllPathsFinderSlave::AllPathsFinderSlave() :
	m_graph(Graph::GetInstance())
{
	int nodesCount = m_graph->GetNodesCount();

	m_viz = new bool[nodesCount];
	m_auxPathStack = new int[nodesCount];
	for (int i = 0; i < nodesCount; ++i)
	{
		m_viz[i] = false;
		m_auxPathStack[i] = 0;
	}
}

void AllPathsFinderSlave::FindAllPaths(int startingNodeIndex, int destinationNodeIndex)
{
	m_startingNodeIndex = startingNodeIndex;
	m_destinationNodeIndex = destinationNodeIndex;

	int msg[2];
	MPI_Status status;
	while (true)
	{
		MPI_Recv(msg, 2, MPI_INT, CommonUtils::GetMasterRank(), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		Slave::Log("Received from master " + CommonUtils::IntsToString(msg, 2));
		if (msg[0] == msg[1] && msg[1] == 0) // receieved 0 0
		{
			break; // the slave was stopped by the slavePool. enters in waiting command mode.
		}
		else
		{
			if (msg[0] != 1)
			{
				CommonUtils::ShowError(-1, "Received unexpected message from master in AllPathsFinderSlave::FindAllPaths");
				break;
			}
			int startingPathLength = msg[1];
			int *startingPath = new int[startingPathLength];
			MPI_Recv(startingPath, startingPathLength, MPI_INT, CommonUtils::GetMasterRank(), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			Slave::Log("Received from master " + CommonUtils::IntsToString(startingPath, startingPathLength));

			FindAllPathsToDestination(startingPathLength, startingPath);

			delete[] startingPath;
		}
	}
	Slave::Log("Slave exited waiting for messages from master. Returning to Slave::Run");
}

const vector<GraphPath*>& AllPathsFinderSlave::GetAllPaths()
{
	return m_dummyAllPaths;
}

void AllPathsFinderSlave::FindAllPathsToDestination(int startingPathLength, int * startingPath)
{
	for (int i = 0; i < startingPathLength; ++i)
	{
		m_auxPathStack[i] = startingPath[i];
		m_viz[startingPath[i]] = true;
	}
	int startingNodeLocal = startingPath[startingPathLength - 1];

	m_viz[startingNodeLocal] = false;

	m_allFoundPaths.clear();
	DFSFindAllPathsToDestination(startingNodeLocal, startingPathLength - 1);
	SendAllPathsToMaster();
	
	int nodesCount = m_graph->GetNodesCount();
	for (int i = 0; i < nodesCount; ++i)
	{
		m_viz[i] = false;
		m_auxPathStack[i] = 0;
	}
}

void AllPathsFinderSlave::DFSFindAllPathsToDestination(int nodeIndex, int stackLevel)
{
	if (m_viz[nodeIndex])
	{
		return;
	}
	m_auxPathStack[stackLevel] = nodeIndex;
	if (nodeIndex == m_destinationNodeIndex)
	{
		m_allFoundPaths.push_back(stackLevel + 1);
		for (int i = 0; i <= stackLevel; ++i)
		{
			m_allFoundPaths.push_back(m_auxPathStack[i]);
		}
		return;
	}

	m_viz[nodeIndex] = true;
	
	Node* node = m_graph->GetNodes()[nodeIndex];
	const vector<Node*>& edges = node->GetEdges();

	for (vector<Node*>::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		DFSFindAllPathsToDestination((*it)->GetIndex(), stackLevel + 1);
	}

	m_viz[nodeIndex] = false;
}

void AllPathsFinderSlave::SendAllPathsToMaster()
{
	int msgLength = m_allFoundPaths.size();
	MPI_Send(&msgLength, 1, MPI_INT, CommonUtils::GetMasterRank(), SEND_MESSAGE_TAG, MPI_COMM_WORLD);
	if (msgLength != 0) 
	{
		MPI_Send(m_allFoundPaths.data(), msgLength, MPI_INT, CommonUtils::GetMasterRank(), SEND_MESSAGE_TAG, MPI_COMM_WORLD);
	}
}

AllPathsFinderSlave::~AllPathsFinderSlave()
{
	if (m_viz != nullptr)
	{
		delete[] m_viz;
	}
	if (m_auxPathStack != nullptr)
	{
		delete[] m_auxPathStack;
	}
}
