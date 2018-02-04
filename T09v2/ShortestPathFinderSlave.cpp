#include "ShortestPathFinderSlave.h"
#include "Graph.h"
#include "GraphPath.h"
#include "Node.h"
#include "CommonUtils.h"
#include "mpi.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

ShortestPathFinderSlave::ShortestPathFinderSlave(int* nodesIntervals, int nodesIntervalsLength, int rank) :
	m_rank(rank),
	m_graph(Graph::GetInstance())
{
	m_nodesIntervals = new int[nodesIntervalsLength];
	for (int i = 0; i < nodesIntervalsLength; ++i)
	{
		m_nodesIntervals[i] = nodesIntervals[i];
	}
	m_globalLowNode = m_nodesIntervals[m_rank * 2];
	m_globalHighNode = m_nodesIntervals[m_rank * 2 + 1];
	m_localLowNode = 0;
	m_localHighNode = m_globalHighNode - m_globalLowNode;
	m_localNodesCount = m_localHighNode - m_localLowNode;

	m_known = new bool[m_localNodesCount];
	m_dist = new int[m_localNodesCount];
	m_pred = new int[m_localNodesCount];

	for (int i = 0; i < m_localNodesCount; ++i)
	{
		m_known[i] = false;
		m_dist[i] = INF;
		m_pred[i] = -1;
	}
}

void ShortestPathFinderSlave::FindShortestPath(int startingNodeIndex, int destinationNodeIndex)
{
	cout << "\n\n\n\n\t\t\tPuLA Slave\n\n\n\n";
	cout.flush();


	m_startingNodeIndex = startingNodeIndex;
	m_destinationNodeIndex = destinationNodeIndex;

	if (IsGlobalNodeHere(startingNodeIndex))
	{
		int localStatingNodeIndex = GlobalToLocal(startingNodeIndex);
		m_known[localStatingNodeIndex] = true;
		m_dist[localStatingNodeIndex] = 0;
	}

	for (int i = 0; i < m_graph->GetNodesCount() - 1; ++i)
	{
		FindLocalMin();

		int localMinMsg[2];
		int globalMinMsg[2];

		if (m_localMin < INF)
		{
			int globalMinPos = LocalToGlobal(m_localMinPos);
			localMinMsg[0] = m_localMin;
			localMinMsg[1] = globalMinPos;
		}
		else
		{
			localMinMsg[0] = INF;
			localMinMsg[1] = INF;
		}

		MPI_Allreduce(localMinMsg, globalMinMsg, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

		int globalMinDist = globalMinMsg[0];
		int globalMinDistPos = globalMinMsg[1];


		if (globalMinDist != INF && globalMinDistPos != INF)
		{
			if (globalMinDistPos == destinationNodeIndex)
			{
				// a path to destination was found. now going to gather those m_preds
				break;
			}
			if (IsGlobalNodeHere(globalMinDistPos))
			{
				m_known[GlobalToLocal(globalMinDistPos)] = true;
			}

			Node* globalMinNode = m_graph->GetNodes()[globalMinDistPos];
			const vector<Node*>& edges = globalMinNode->GetEdges();
			for (vector<Node*>::const_iterator it = edges.begin(); it != edges.end(); ++it)
			{
				Node* adjacentNode = *it;
				if (IsGlobalNodeHere(adjacentNode->GetIndex()))
				{
					int localAdjacentNodeIndex = GlobalToLocal(adjacentNode->GetIndex());
					if (!m_known[localAdjacentNodeIndex])
					{
						int blocksBetween = Node::GetBlocksBetween(*globalMinNode, *adjacentNode);
						if (globalMinDist + blocksBetween < m_dist[localAdjacentNodeIndex])
						{
							m_dist[localAdjacentNodeIndex] = globalMinDist + blocksBetween;
							m_pred[localAdjacentNodeIndex] = globalMinNode->GetIndex();
						}
					}
				}
			}
		}
		else
		{
			break;
		}
	}

	GatherResultsInMaster();
}


ShortestPathFinderSlave::~ShortestPathFinderSlave()
{
	delete[] m_nodesIntervals;
	delete[] m_known;
	delete[] m_dist;
	delete[] m_pred;
}

void ShortestPathFinderSlave::FindLocalMin()
{
	m_localMin = INF;
	m_localMinPos = INF;
	for (int i = 0; i < m_localNodesCount; ++i)
	{
		if (!m_known[i])
		{
			if (m_dist[i] < m_localMin)
			{
				m_localMin = m_dist[i];
				m_localMinPos = i;
			}
		}
	}
}

int ShortestPathFinderSlave::LocalToGlobal(int nodeIndex)
{
	return nodeIndex + m_localLowNode;
}

int ShortestPathFinderSlave::GlobalToLocal(int nodeIndex)
{
	return nodeIndex - m_localLowNode;
}

bool ShortestPathFinderSlave::IsGlobalNodeHere(int globalNodeIndex)
{
	return m_globalLowNode <= globalNodeIndex && globalNodeIndex < m_globalHighNode;
}

void ShortestPathFinderSlave::GatherResultsInMaster()
{
	int infinity = INF;

	int* distancesToSend;
	int distancesLocalCount;
	int* predsToSend;
	int predsLocalCount;

	if (m_localNodesCount == 0)
	{
		distancesToSend = &infinity;
		distancesLocalCount = 1;

		predsToSend = &infinity;
		predsLocalCount = 1;
	}
	else
	{
		distancesToSend = m_dist;
		distancesLocalCount = m_localNodesCount;

		predsToSend = m_pred;
		predsLocalCount = m_localNodesCount;
	}

	int recvLength = max(m_graph->GetNodesCount(), CommonUtils::GetNrProcesses());
	int* gatheredDist = new int[recvLength];
	int* gatheredPred = new int[recvLength];

	MPI_Gather(distancesToSend, distancesLocalCount, MPI_INT, gatheredDist, recvLength, MPI_INT, CommonUtils::GetMasterRank(), MPI_COMM_WORLD);
	MPI_Gather(predsToSend, predsLocalCount, MPI_INT, gatheredPred, recvLength, MPI_INT, CommonUtils::GetMasterRank(), MPI_COMM_WORLD);

	delete[] gatheredDist;
	delete[] gatheredPred;
}
