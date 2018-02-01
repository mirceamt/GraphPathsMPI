#pragma once
#include <string>

class Graph;

class Slave
{
public:
	Slave(int rank);
	void Init();
	void Run();
	void CleanUp();
	void InitGraph();
	static void Log(std::string s);

private:
	void FindAllPaths();
	void HandleErrorsOfBcastedCommand(int errorCode, int *message);

	const int m_masterRank;
	Graph* m_graph;
	static int m_rank;
};