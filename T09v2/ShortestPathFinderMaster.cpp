#include "ShortestPathFinderMaster.h"
#include "Graph.h"
#include "GraphPath.h"
#include "Node.h"
#include "CommonUtils.h"
#include "mpi.h"
#include <iostream>
#include <vector>

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

void ShortestPathFinderMaster::FindShortestPath(int startingNodeIndex, int destinationNodeIndex)
{
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
			m_known[GlobalToLocal(globalMinDistPos)] = true;
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
	}

	// TODO: gather m_dist, m_pred in master
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
	for (int i = 0; m_localNodesCount; ++i)
	{
		if (m_dist[i] < m_localMin)
		{
			m_localMin = m_dist[i];
			m_localMinPos = i;
		}
	}
}

int ShortestPathFinderMaster::LocalToGlobal(int nodeIndex)
{
	return nodeIndex + m_localLowNode;
}

int ShortestPathFinderMaster::GlobalToLocal(int nodeIndex)
{
	return nodeIndex - m_localLowNode;
}

bool ShortestPathFinderMaster::IsGlobalNodeHere(int globalNodeIndex)
{
	return m_globalLowNode <= globalNodeIndex && globalNodeIndex < m_globalHighNode;
}
