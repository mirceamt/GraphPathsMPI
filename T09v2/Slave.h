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
	static void Log(std::string s);
	static int Rank();

private:
	void InitGraph();
	void FindAllPaths();
	void FindLongestPath();
	void HandleErrorsOfBcastedCommand(int errorCode, int *message);

	const int m_masterRank;
	Graph* m_graph;
	static int m_rank;
};