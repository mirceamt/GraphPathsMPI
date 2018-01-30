#include "Master.h"
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include "Graph.h"
#include "CommonUtils.h"
#include "mpi.h"

using namespace std;

Master::Master(int rank) :
	m_graph(Graph::GetInstance()),
	m_rank(rank)
{ }

void Master::Init()
{
	cout << "Initalized master process with rank " << m_rank << "\n";
	cout.flush();
}

void Master::Run()
{
	cout << "Entered Master::Run from master with rank " << m_rank << "\n";
	cout.flush();
	int option = -1;
	while (option != 0)
	{
		ShowMenu();
		cin >> option;
		BroadcastOption(option);
		switch(option)
		{
		case 0:
			break;
		case 1: 
			InitGraph();
			break;
		case 2:
			FindAllPaths();
			break;
		case 3:
			//FindShortestPath();
			break;
		case 4:
			//FindLongestPath();
			break;
		default:
			break;
		}
		ClearScreen();

	}
}

void Master::CleanUp()
{
	Log("Exiting process " + CommonUtils::IntToString(m_rank));
}

void Master::InitGraph()
{
	Log("Entered InitGraph");

	m_graph->Reset();
	Log("Reseted graph");

	int message[4];
	memset(message, 0, 4 * sizeof(int));

	ifstream auxHelp("..\\Resources\\inputFileName.txt");
	char a[MAX_PATH];
	auxHelp.getline(a, MAX_PATH);
	auxHelp.close();
	ClearScreen();
	ShowInitGraphText(a);

	ifstream inputFile(a);
	
	int nrLines;
	inputFile >> nrLines;
	MPI_Bcast(&nrLines, 1, MPI_INT, m_rank, MPI_COMM_WORLD); // m_rank = 0; broadcasting the number of lines in the input file

	int streetNumber, crossStreet1, crossStreet2, streetType;
	for (int i = 1; i <= nrLines; ++i)
	{
		inputFile >> streetNumber >> crossStreet1 >> crossStreet2 >> streetType;
		message[0] = streetNumber;
		message[1] = crossStreet1;
		message[2] = crossStreet2;
		message[3] = streetType;
		MPI_Bcast(message, 4, MPI_INT, m_rank, MPI_COMM_WORLD); // m_rank = 0; broadcasting a line from the input file, meaning a street between 2 intersections

		m_graph->AddEdgeAndNodes(streetNumber, crossStreet1, crossStreet2, streetType);
	}
	inputFile.close();

	Log("Exited InitGraph");
}

void Master::BroadcastOption(int option)
{
	int message[2];
	memset(message, 0, 2 * sizeof(int));
	message[0] = -1; // -1 means that we are sending a menu option 
	message[1] = option; // message[1] is the chosen option
	int error = MPI_Bcast(message, 2, MPI_INT, m_rank, MPI_COMM_WORLD); // m_rank should always be 0 as this code runs in the master process
	if (error != MPI_SUCCESS)
	{
		CommonUtils::ShowError(error, "Error broadcasting \"-1, 1\" from Master in InitGraph");
		exit(0);
	}
}

void Master::FindAllPaths()
{
	
}


void Master::ShowInitGraphText(char *s)
{
	cout << "\nReading from init path: " << s << "\n";
	cout.flush();
}

void Master::ShowMenu()
{
	cout << "\n";
	cout << "\t\tMeniu";
	cout << "\n";
	cout << "\n\t1. Initializare graf";
	cout << "\n\t2. Toate drumurile";
	cout << "\n\t3. Drumul minim";
	cout << "\n\t4. Drumul maxim";
	cout << "\n\n\t0. Iesire";
	cout << "\n\n\n  Alegeti o optiune: ";
	cout.flush();
}

void Master::Log(string s)
{
	cout << "Master " << m_rank << ": ";
	cout << s;
	cout << "\n";
	cout.flush();
}

void Master::ClearScreen()
{
	for (int i = 1; i <= 10; ++i)
	{
		cout << "\n";
	}
	cout.flush();
	//system("CLS");
}
