#include "Master.h"
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include "Graph.h"
#include "CommonUtils.h"
#include "IAllPathsFinder.h"
#include "AllPathsFinderMaster.h"
#include "ILongestPathFinder.h"
#include "LongestPathFinderMaster.h"
#include "IShortestPathFinder.h"
#include "ShortestPathFinderMaster.h"
#include "mpi.h"

using namespace std;

int Master::m_rank;

Master::Master(int rank) :
	m_graph(Graph::GetInstance())
{ 
	m_rank = rank;
}

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
			FindShortestPath();
			break;
		case 4:
			FindLongestPath();
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

	Log("\n\n\n-----------------------------------\n\n\n");

	cout << "map size: ";
	cout << m_graph->GetMapStreetsToIndex().size();
	cout.flush();
	for (auto x : m_graph->GetMapStreetsToIndex())
	{
		cout << "(" << x.first.first << ", " << x.first.second << ") - " << x.second << "\n";
		cout.flush();
	}

	Log("\n\n\n-----------------------------------\n\n\n");

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

void Master::ComputeNodeIntervals(int *nodeIntervals)
{
	int nrProc = CommonUtils::GetNrProcesses();
	int nrNodes = m_graph->GetNodesCount();
	int chunk = nrNodes / nrProc;
	int remainder = nrNodes % nrProc;
	const int msgSize = nrProc * 2;

	int lastNode = 0;
	for (int i = 0; i < nrProc; ++i)
	{
		int actualChunk = chunk;
		if (remainder > 0)
		{
			--remainder;
			actualChunk += 1;
		}
		nodeIntervals[2 * i] = lastNode;
		nodeIntervals[2 * i + 1] = lastNode + actualChunk;
		lastNode += actualChunk;
	}
}

void Master::FindAllPaths()
{
	int msg[4];
	int WEStreet, NSStreet;
	//get the starting intersection
	cout << "\n";
	cout << "Enter the street numbers of the starting intersection";
	cout << "\n";
	cout << "\tEnter West-East Street: ";
	cout.flush();
	cin >> WEStreet;
	cout << "\tEnter North-South Street: ";
	cout.flush();
	cin >> NSStreet;
	msg[0] = WEStreet; 
	msg[1] = NSStreet;

	pair<int, int> startingIntersection = make_pair(WEStreet, NSStreet);
	if (!m_graph->ExistsNodeInGraph(startingIntersection))
	{
		cout << "There is no such starting intersection in the city.";
		msg[0] = msg[1] = msg[2] = msg[3] = -1;
		MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD);
		return;
	}

	//get the destination intersection
	cout << "\n";
	cout << "Enter the street numbers of the destination intersection";
	cout << "\n";
	cout << "\tEnter West-East Street: ";
	cout.flush();
	cin >> WEStreet;
	cout << "\tEnter North-South Street: ";
	cout.flush();
	cin >> NSStreet;
	msg[2] = WEStreet;
	msg[3] = NSStreet;

	pair<int, int> destinationIntersection = make_pair(WEStreet, NSStreet);

	if (!m_graph->ExistsNodeInGraph(destinationIntersection))
	{
		cout << "There is no such destiantion intersection in the city.";
		msg[0] = msg[1] = msg[2] = msg[3] = -1;
		MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD);
		return;
	}
	MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD); // send the starting and destination intersection to slaves

	int startingIntersectionIndex = m_graph->GetIntersectionIndex(startingIntersection);
	int destinationIntersetionIndex = m_graph->GetIntersectionIndex(destinationIntersection);
	IAllPathsFinder* allPathsFinder = new AllPathsFinderMaster();
	allPathsFinder->FindAllPaths(startingIntersectionIndex, destinationIntersetionIndex);
	
	//const vector<GraphPath*>& allPaths = allPathsFinder->GetAllPaths();
	AllPathsFinderMaster* allPathsFinderMaster = dynamic_cast<AllPathsFinderMaster*>(allPathsFinder);
	if (allPathsFinderMaster != nullptr)
	{
		allPathsFinderMaster->ShowAllPaths();
	}
	else
	{
		cout << "\n\n\tERROR!!!";
		cout << "\n\tdynamic_cast failed in Master::FindAllPaths";
		cout.flush();
	}

	delete allPathsFinder; // at this point all the GraphPath objects will be gone.
}

void Master::FindShortestPath()
{
	int msg[4];
	int WEStreet, NSStreet;
	//get the starting intersection
	cout << "\n";
	cout << "Enter the street numbers of the starting intersection";
	cout << "\n";
	cout << "\tEnter West-East Street: ";
	cout.flush();
	cin >> WEStreet;
	cout << "\tEnter North-South Street: ";
	cout.flush();
	cin >> NSStreet;
	msg[0] = WEStreet;
	msg[1] = NSStreet;

	pair<int, int> startingIntersection = make_pair(WEStreet, NSStreet);
	if (!m_graph->ExistsNodeInGraph(startingIntersection))
	{
		cout << "There is no such starting intersection in the city.";
		msg[0] = msg[1] = msg[2] = msg[3] = -1;
		MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD);
		return;
	}

	//get the destination intersection
	cout << "\n";
	cout << "Enter the street numbers of the destination intersection";
	cout << "\n";
	cout << "\tEnter West-East Street: ";
	cout.flush();
	cin >> WEStreet;
	cout << "\tEnter North-South Street: ";
	cout.flush();
	cin >> NSStreet;
	msg[2] = WEStreet;
	msg[3] = NSStreet;

	pair<int, int> destinationIntersection = make_pair(WEStreet, NSStreet);

	if (!m_graph->ExistsNodeInGraph(destinationIntersection))
	{
		cout << "There is no such destiantion intersection in the city.";
		msg[0] = msg[1] = msg[2] = msg[3] = -1;
		MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD);
		return;
	}
	MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD); // send the starting and destination intersection to slaves

	int nodeIntervalsLength = CommonUtils::GetNrProcesses() * 2;
	int *nodeIntervals = new int[nodeIntervalsLength];
	ComputeNodeIntervals(nodeIntervals);
	MPI_Bcast(nodeIntervals, nodeIntervalsLength, MPI_INT, CommonUtils::GetMasterRank(), MPI_COMM_WORLD);

	int startingIntersectionIndex = m_graph->GetIntersectionIndex(startingIntersection);
	int destinationIntersectionIndex = m_graph->GetIntersectionIndex(destinationIntersection);

	IShortestPathFinder *shortestPathFinder = new ShortestPathFinderMaster(nodeIntervals, nodeIntervalsLength, m_rank);
	shortestPathFinder->FindShortestPath(startingIntersectionIndex, destinationIntersectionIndex);
	
	ShortestPathFinderMaster* shortestPathFinderMaster = dynamic_cast<ShortestPathFinderMaster*>(shortestPathFinder);
	if (shortestPathFinderMaster != nullptr)
	{
		shortestPathFinderMaster->ShowShortestPath();
	}

	delete[] nodeIntervals;
	delete shortestPathFinder;
}

void Master::FindLongestPath()
{
	int msg[4];
	int WEStreet, NSStreet;
	//get the starting intersection
	cout << "\n";
	cout << "Enter the street numbers of the starting intersection";
	cout << "\n";
	cout << "\tEnter West-East Street: ";
	cout.flush();
	cin >> WEStreet;
	cout << "\tEnter North-South Street: ";
	cout.flush();
	cin >> NSStreet;
	msg[0] = WEStreet;
	msg[1] = NSStreet;

	pair<int, int> startingIntersection = make_pair(WEStreet, NSStreet);
	if (!m_graph->ExistsNodeInGraph(startingIntersection))
	{
		cout << "There is no such starting intersection in the city.";
		msg[0] = msg[1] = msg[2] = msg[3] = -1;
		MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD);
		return;
	}

	//get the destination intersection
	cout << "\n";
	cout << "Enter the street numbers of the destination intersection";
	cout << "\n";
	cout << "\tEnter West-East Street: ";
	cout.flush();
	cin >> WEStreet;
	cout << "\tEnter North-South Street: ";
	cout.flush();
	cin >> NSStreet;
	msg[2] = WEStreet;
	msg[3] = NSStreet;

	pair<int, int> destinationIntersection = make_pair(WEStreet, NSStreet);

	if (!m_graph->ExistsNodeInGraph(destinationIntersection))
	{
		cout << "There is no such destiantion intersection in the city.";
		msg[0] = msg[1] = msg[2] = msg[3] = -1;
		MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD);
		return;
	}
	MPI_Bcast(msg, 4, MPI_INT, m_rank, MPI_COMM_WORLD); // send the starting and destination intersection to slaves

	int startingIntersectionIndex = m_graph->GetIntersectionIndex(startingIntersection);
	int destinationIntersetionIndex = m_graph->GetIntersectionIndex(destinationIntersection);

	ILongestPathFinder* longestPathFinder = new LongestPathFinderMaster();
	longestPathFinder->FindLongestPath(startingIntersectionIndex, destinationIntersetionIndex);

	LongestPathFinderMaster* longestPathFinderMaster = dynamic_cast<LongestPathFinderMaster*>(longestPathFinder);
	if (longestPathFinderMaster != nullptr)
	{
		longestPathFinderMaster->ShowLongestPath();
	}
	else
	{
		cout << "\n\n\tERROR!!!";
		cout << "\n\tdynamic_cast failed in Master::FindLongestPath";
		cout.flush();
	}

	delete longestPathFinder; // at this point the longest GraphPath will be gone
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
	cout << "Master " << CommonUtils::GetMasterRank() << ": ";
	cout << s;
	cout << "\n";
	cout.flush();
}

int Master::Rank()
{
	return m_rank;
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
