#include "Slave.h"
#include <vector>
#include <map>
#include <iostream>
#include "mpi.h"
#include "Graph.h"
#include "Node.h"
#include "IAllPathsFinder.h"
#include "AllPathsFinderSlave.h"
#include "CommonUtils.h"

using namespace std;

Slave::Slave(int rank) :
	m_graph(Graph::GetInstance()), 
	m_rank(rank), 
	m_masterRank(0)
{ }

void Slave::Init()
{
	cout << "Initalized slave process with rank " << m_rank << "\n";
	cout.flush();
}

void Slave::Run()
{
	cout << "Entered Slave::Run from salve with rank " << m_rank << "\n";
	cout.flush();
	bool done = false;
	int message[2];

	while (!done)
	{
		int errorCode = MPI_Bcast(message, 2, MPI_INT, m_masterRank, MPI_COMM_WORLD);
		Log("Received from master: " + CommonUtils::IntsToString(message, 2));
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
			FindAllPaths();
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
	Log("Exiting process " + CommonUtils::IntToString(m_rank));
}

void Slave::FindAllPaths()
{
	int msg[4];
	MPI_Bcast(msg, 4, MPI_INT, m_masterRank, MPI_COMM_WORLD); // receive the starting and destiantion intersection from master
	if (msg[0] == msg[1] && msg[1] == msg[2] && msg[2] == msg[3] && msg[3] == -1)
	{
		CommonUtils::ShowError(-1, "ERROR! Received bad interserctions from master");
		return;
	}

	pair<int, int> startingIntersection, destinationIntersection;
	startingIntersection.first = msg[0]; 
	startingIntersection.second = msg[1];
	destinationIntersection.first = msg[2];
	destinationIntersection.second = msg[3];

	int startingNodeIndex = m_graph->GetIntersectionIndex(startingIntersection);
	int destinationNodeIndex = m_graph->GetIntersectionIndex(destinationIntersection);

	IAllPathsFinder* allPathsFinder = new AllPathsFinderSlave();
	allPathsFinder->FindAllPaths(startingNodeIndex, destinationNodeIndex);
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

void Slave::Log(string s)
{
	cout << "Slave " << m_rank << ": ";
	cout << s.c_str();
	cout << "\n";
	cout.flush();
}

void Slave::InitGraph()
{
	Log("Entered InitGraph");

	m_graph->Reset();
	Log("Reseted graph");

	int nrLines = 0;
	MPI_Bcast(&nrLines, 1, MPI_INT, m_masterRank, MPI_COMM_WORLD);
	Log("Received broadcasted msg from master: " + CommonUtils::IntsToString(&nrLines, 1));

	int message[4];
	for (int i = 1; i <= nrLines; ++i)
	{
		MPI_Bcast(message, 4, MPI_INT, m_masterRank, MPI_COMM_WORLD); // m_rank = 0; receiving a broadcasted line from the input file, meaning a street between 2 intersection
		Log("Received broadcasted msg from master: " + CommonUtils::IntsToString(message, 4));
		m_graph->AddEdgeAndNodes(message[0], message[1], message[2], message[3]);
	}
	Log("Exited InitGraph");
}
