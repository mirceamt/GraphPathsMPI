#include "Master.h"
#include <iostream>
#include <Windows.h>
#include "Graph.h"

using namespace std;

Master::Master() :
	m_graph(Graph::GetInstance())
{

}

void Master::Run()
{
	int option = -1;
	while (option != 0)
	{
		ClearScreen();
		ShowMenu();
		cin >> option;
		switch(option)
		{
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
	}
}

void Master::InitGraph()
{
	ClearScreen();
	ShowInitGraphText();
	char a[MAX_PATH];
	cin.getline(a, MAX_PATH);
	/// TODO
}

void Master::FindAllPaths()
{
	
}


void Master::ShowInitGraphText()
{
	cout << "\n\tInsert init path: ";
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
}

void Master::ClearScreen()
{
	for (int i = 1; i <= 100; ++i)
	{
		cout << "\n";
	}
}
