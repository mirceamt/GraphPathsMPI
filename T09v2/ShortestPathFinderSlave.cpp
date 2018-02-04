#include "ShortestPathFinderSlave.h"
#include "Graph.h"
#include "GraphPath.h"
#include "Node.h"
#include "CommonUtils.h"
#include "mpi.h"
#include "Slave.h"
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

	m_roundedNodesCount = m_nodesIntervals[1] - m_nodesIntervals[0];

	m_known = new bool[m_roundedNodesCount];
	m_dist = new int[m_roundedNodesCount];
	m_pred = new int[m_roundedNodesCount];

	for (int i = 0; i < m_roundedNodesCount; ++i)
	{
		m_known[i] = false;
		m_dist[i] = INF;
		m_pred[i] = -1;
	}
}

void ShortestPathFinderSlave::FindShortestPath(int startingNodeIndex, int destinationNodeIndex)
{
	Slave::Log("\nlocalLow: " + CommonUtils::IntToString(m_localLowNode) + "\n" +
		"localHigh: " + CommonUtils::IntToString(m_localHighNode) + "\n" +
		"globalLow: " + CommonUtils::IntToString(m_globalLowNode) + "\n" +
		"globalHigh: " + CommonUtils::IntToString(m_globalHighNode) + "\n"
	);

	m_startingNodeIndex = startingNodeIndex;
	m_destinationNodeIndex = destinationNodeIndex;

	if (IsGlobalNodeHere(startingNodeIndex))
	{
		int localStatingNodeIndex = GlobalToLocal(startingNodeIndex);
		m_known[localStatingNodeIndex] = true;
		m_dist[localStatingNodeIndex] = 0;
		Slave::Log("Starting Node " + CommonUtils::IntToString(startingNodeIndex) + " is here");
		Slave::Log("m_known: " + CommonUtils::BoolsToString(m_known, m_localNodesCount));
		Slave::Log("m_dist: " + CommonUtils::IntsToString(m_dist, m_localNodesCount));
	}

	for (int i = 0; i < m_graph->GetNodesCount() - 1; ++i)
	{
		//Slave::Log("Step " + CommonUtils::IntToString(i));
		FindLocalMin();
		//Slave::Log("Min found: " + CommonUtils::IntToString(m_localMin));
		//Slave::Log("Min position found: " + CommonUtils::IntToString(m_localMinPos));

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
	Slave::Log("m_dist: " + CommonUtils::IntsToString(m_dist, m_roundedNodesCount));
	Slave::Log("m_pred: " + CommonUtils::IntsToString(m_pred, m_roundedNodesCount));
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
	return nodeIndex + m_globalLowNode;
}

int ShortestPathFinderSlave::GlobalToLocal(int nodeIndex)
{
	return nodeIndex - m_globalLowNode;
}

bool ShortestPathFinderSlave::IsGlobalNodeHere(int globalNodeIndex)
{
	return m_globalLowNode <= globalNodeIndex && globalNodeIndex < m_globalHighNode;
}

void ShortestPathFinderSlave::GatherResultsInMaster()
{
	int infinity = INF;

	int* distancesToSend = m_dist;
	int* predsToSend = m_pred;

	for (int i = m_localNodesCount; i < m_roundedNodesCount; ++i)
	{
		distancesToSend[i] = -2;
		predsToSend[i] = -2;
	}

	int recvLength = m_roundedNodesCount * CommonUtils::GetNrProcesses();

	int* gatheredDist = new int[recvLength];
	int* gatheredPred = new int[recvLength];
	for (int i = 0; i < recvLength; ++i)
	{
		gatheredDist[i] = gatheredPred[i] = 0;
	}

	int rez1 = MPI_Gather(distancesToSend, m_roundedNodesCount, MPI_INT, gatheredDist, m_roundedNodesCount, MPI_INT, CommonUtils::GetMasterRank(), MPI_COMM_WORLD);
	int rez2 = MPI_Gather(predsToSend, m_roundedNodesCount, MPI_INT, gatheredPred, m_roundedNodesCount, MPI_INT, CommonUtils::GetMasterRank(), MPI_COMM_WORLD);

	delete[] gatheredDist;
	delete[] gatheredPred;
}
