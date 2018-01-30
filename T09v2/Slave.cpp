#include "Slave.h"
#include <vector>
#include <map>
#include <iostream>
#include "mpi.h"
#include "Graph.h"
#include "Node.h"
#include "CommonUtils.h"

using namespace std;

Slave::Slave(int rank) :
	m_graph(Graph::GetInstance()), 
	m_rank(rank), 
	m_masterRank(0)
{ }

void Slave::Init()
{
	//TODO?
}

void Slave::Run()
{
	bool done = false;
	int message[2];

	while (!done)
	{
		int errorCode = MPI_Bcast(message, 2, MPI_INT, m_masterRank, MPI_COMM_WORLD);
		HandleErrorsOfBcastedCommand(errorCode, message);
		switch (message[1])
		{
		case 0:
			done = true;
			break;
		case 1:
			InitGraph();
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;

		default:
			break;
		}

	}
}

void Slave::CleanUp()
{
	//TODO?
}

void Slave::HandleErrorsOfBcastedCommand(int errorCode, int *message)
{
	if (errorCode != MPI_SUCCESS)
	{
		CommonUtils::ShowError(errorCode, "Error in receiving broadcasted command in Slave::Run");
		exit(0);
	}
	if (message[0] != -1)
	{
		CommonUtils::ShowError(-1, "Broadcasted message does not start with -1 in Slave::Run");
		exit(0);
	}
	if (message[1] < 0 || message[1] > 4)
	{
		CommonUtils::ShowError(-1, "Broadcasted command is not between [0, 4] in Slave::Run");
		exit(0);
	}
}

void Slave::InitGraph()
{
	m_graph->Reset();
	int nrLines = 0;
	MPI_Bcast(&nrLines, 1, MPI_INT, m_masterRank, MPI_COMM_WORLD);

	int message[4];
	for (int i = 1; i <= nrLines; ++i)
	{
		MPI_Bcast(message, 4, MPI_INT, m_masterRank, MPI_COMM_WORLD); // m_rank = 0; receiving a broadcasted line from the input file, meaning a street between 2 intersection
		m_graph->AddEdgeAndNodes(message[0], message[1], message[2], message[3]);
	}
}
