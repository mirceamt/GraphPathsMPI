#include "LongestPathFinderSlave.h"
#include "Graph.h"
#include "GraphPath.h"
#include "Node.h"
#include "Slave.h"
#include <vector>
#include "mpi.h"
#include "CommonUtils.h"

using namespace std;

LongestPathFinderSlave::LongestPathFinderSlave() :
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

void LongestPathFinderSlave::FindLongestPath(int startingNodeIndex, int destinationNodeIndex)
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
				CommonUtils::ShowError(-1, "Received unexpected message from master in LongestPathFinderSlave::FindLongestPath");
				break;
			}
			int startingPathLength = msg[1];
			int *startingPath = new int[startingPathLength];
			MPI_Recv(startingPath, startingPathLength, MPI_INT, CommonUtils::GetMasterRank(), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			Slave::Log("Received from master " + CommonUtils::IntsToString(startingPath, startingPathLength));

			FindLongestPathToDestination(startingPathLength, startingPath);

			delete[] startingPath;
		}
	}
	Slave::Log("Slave exited waiting for messages from master. Returning to Slave::Run");
}

GraphPath* LongestPathFinderSlave::GetLongestPath()
{
	return m_longestPath;
}

void LongestPathFinderSlave::FindLongestPathToDestination(int startingPathLength, int * startingPath)
{
	int startingPathBlocksDistance = GraphPath(startingPathLength, startingPath).GetBlocksLength();
	for (int i = 0; i < startingPathLength; ++i)
	{
		m_auxPathStack[i] = startingPath[i];
		m_viz[startingPath[i]] = true;
	}
	int startingNodeLocal = startingPath[startingPathLength - 1];

	m_viz[startingNodeLocal] = false;

	if (m_longestPath != nullptr)
	{
		delete m_longestPath;
	}
	m_longestPath = nullptr;

	DFSFindLongestPathToDestination(startingNodeLocal, startingPathLength - 1, startingPathBlocksDistance);
	SendLongestPathToMaster();

	int nodesCount = m_graph->GetNodesCount();
	for (int i = 0; i < nodesCount; ++i)
	{
		m_viz[i] = false;
		m_auxPathStack[i] = 0;
	}
}

void LongestPathFinderSlave::DFSFindLongestPathToDestination(int nodeIndex, int stackLevel, int blocksLength)
{
	if (m_viz[nodeIndex])
	{
		return;
	}
	m_auxPathStack[stackLevel] = nodeIndex;
	if (nodeIndex == m_destinationNodeIndex)
	{
		if (m_longestPath == nullptr || m_longestPath->GetBlocksLength() < blocksLength)
		{
			if (m_longestPath != nullptr)
			{
				delete m_longestPath;
			}
			m_longestPath = new GraphPath(stackLevel + 1, m_auxPathStack);
		}
		return;
	}

	m_viz[nodeIndex] = true;

	Node* node = m_graph->GetNodes()[nodeIndex];
	const vector<Node*>& edges = node->GetEdges();

	for (vector<Node*>::const_iterator it = edges.begin(); it != edges.end(); ++it)
	{
		int newBlocksLength = Node::GetBlocksBetween(*node, *(*it));
		DFSFindLongestPathToDestination((*it)->GetIndex(), stackLevel + 1, blocksLength + newBlocksLength);
	}

	m_viz[nodeIndex] = false;
}

void LongestPathFinderSlave::SendLongestPathToMaster()
{
	if (m_longestPath != nullptr)
	{
		int msg[2];
		int nodesCount = m_longestPath->GetCount();
		int blocksLength = m_longestPath->GetBlocksLength();

		msg[0] = nodesCount;
		msg[1] = blocksLength;
		MPI_Send(msg, 2, MPI_INT, CommonUtils::GetMasterRank(), SEND_MESSAGE_TAG, MPI_COMM_WORLD);

		int *nodesIndices = new int[nodesCount];
		Node** nodes = m_longestPath->GetNodes();

		for (int i = 0; i < nodesCount; ++i)
		{
			nodesIndices[i] = nodes[i]->GetIndex();
		}
		MPI_Send(nodesIndices, nodesCount, MPI_INT, CommonUtils::GetMasterRank(), SEND_MESSAGE_TAG, MPI_COMM_WORLD);

		delete[] nodesIndices;
	}
	else
	{
		int msg[2];
		msg[0] = msg[1] = 0;
		MPI_Send(msg, 2, MPI_INT, CommonUtils::GetMasterRank(), SEND_MESSAGE_TAG, MPI_COMM_WORLD);
	}
}

LongestPathFinderSlave::~LongestPathFinderSlave()
{
	if (m_viz != nullptr)
	{
		delete[] m_viz;
	}
	if (m_auxPathStack != nullptr)
	{
		delete[] m_auxPathStack;
	}
	if (m_longestPath != nullptr)
	{
		delete m_longestPath;
	}
}
