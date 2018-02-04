#include "ShortestPathFinderMaster.h"
#include "Graph.h"
#include "GraphPath.h"
#include "Node.h"
#include "CommonUtils.h"
#include "mpi.h"
#include "Master.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

ShortestPathFinderMaster::ShortestPathFinderMaster(int* nodesIntervals, int nodesIntervalsLength, int rank) :
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

void ShortestPathFinderMaster::FindShortestPath(int startingNodeIndex, int destinationNodeIndex)
{
	Master::Log("\nlocalLow: " + CommonUtils::IntToString(m_localLowNode) + "\n" +
		"localHigh: " + CommonUtils::IntToString(m_localHighNode) + "\n" +
		"globalLow: " + CommonUtils::IntToString(m_globalLowNode) + "\n" +
		"globalHigh: " + CommonUtils::IntToString(m_globalHighNode) + "\n"
	);

	m_startingNodeIndex = startingNodeIndex;
	m_destinationNodeIndex = destinationNodeIndex;

	if (IsGlobalNodeHere(startingNodeIndex))
	{
		int localStatingNodeIndex = GlobalToLocal(startingNodeIndex);
		//m_known[localStatingNodeIndex] = true;
		m_dist[localStatingNodeIndex] = 0;
		Master::Log("Starting Node " + CommonUtils::IntToString(startingNodeIndex) + " is here");
		Master::Log("m_known: " + CommonUtils::BoolsToString(m_known, m_localNodesCount));
		Master::Log("m_dist: " + CommonUtils::IntsToString(m_dist, m_localNodesCount));
	}

	for (int i = 0; i < m_graph->GetNodesCount() - 1; ++i)
	{
		//Master::Log("Step " + CommonUtils::IntToString(i));
		FindLocalMin();
		//Master::Log("Min found: " + CommonUtils::IntToString(m_localMin));
		//Master::Log("Min position found: " + CommonUtils::IntToString(m_localMinPos));


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
	Master::Log("m_dist: " + CommonUtils::IntsToString(m_dist, m_roundedNodesCount));
	Master::Log("m_pred: " + CommonUtils::IntsToString(m_pred, m_roundedNodesCount));

	GatherResultsInMaster();
}

void ShortestPathFinderMaster::ShowShortestPath()
{
	cout << "-------The longest path from ";
	m_graph->GetNodes()[m_startingNodeIndex]->ShowNode(false);
	cout << " to ";
	m_graph->GetNodes()[m_destinationNodeIndex]->ShowNode(false);
	cout << "\n";

	if (m_shortestPath == nullptr)
	{
		cout << "\nNo path found!\n";
		cout.flush();
		return;
	}

	cout << "-------Nodes length: " << m_shortestPath->GetCount() << "\n";
	cout << "-------Blocks length: " << m_shortestPath->GetBlocksLength() << "\n";
	cout << "-------";
	m_shortestPath->ShowNodes();
	cout << "\n";
	cout.flush();
}

ShortestPathFinderMaster::~ShortestPathFinderMaster()
{
	delete[] m_nodesIntervals;
	delete[] m_known;
	delete[] m_dist;
	delete[] m_pred;
	if (m_shortestPath != nullptr)
	{
		delete m_shortestPath;
	}
}

void ShortestPathFinderMaster::FindLocalMin()
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

int ShortestPathFinderMaster::LocalToGlobal(int nodeIndex)
{
	return nodeIndex + m_globalLowNode;
}

int ShortestPathFinderMaster::GlobalToLocal(int nodeIndex)
{
	return nodeIndex - m_globalLowNode;
}

bool ShortestPathFinderMaster::IsGlobalNodeHere(int globalNodeIndex)
{
	return m_globalLowNode <= globalNodeIndex && globalNodeIndex < m_globalHighNode;
}

void ShortestPathFinderMaster::GatherResultsInMaster()
{
	int infinity = INF;

	int* distancesToSend = m_dist;
	int* predsToSend = m_pred;

	// add -2 to fill in the gap to m_roundedNodesCount
	// to assure that MPI_Gather doesn't crash 
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
	
	Master::Log("\n\tGathered pred: " + CommonUtils::IntsToString(gatheredPred, recvLength));
	Master::Log("\n\tGathered dist: " + CommonUtils::IntsToString(gatheredDist, recvLength));

	int k = 0;
	for (int i = 0; i < recvLength; ++i) // remove -2;
	{
		if (gatheredDist[i] == -2) // gatheredPred[i] is also -1
		{
			++k;
		}
		else
		{
			gatheredDist[i - k] = gatheredDist[i];
			gatheredPred[i - k] = gatheredPred[i];
		}
	}
	recvLength -= k;

	if (gatheredPred[m_destinationNodeIndex] != -1)
	{
		vector<int> shortestPathIndices;
		int currentNodeIndex = m_destinationNodeIndex;
		while (currentNodeIndex != -1)
		{
			shortestPathIndices.push_back(currentNodeIndex);
			currentNodeIndex = gatheredPred[currentNodeIndex];
		}
		reverse(shortestPathIndices.begin(), shortestPathIndices.end());
		m_shortestPath = new GraphPath(shortestPathIndices.size(), shortestPathIndices.data());
	}

	delete[] gatheredDist;
	delete[] gatheredPred;
}
