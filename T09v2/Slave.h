#pragma once

class Graph;

class Slave
{
public:
	Slave(int rank);
	void Init();
	void Run();
	void CleanUp();
	void InitGraph();


private:
	void HandleErrorsOfBcastedCommand(int errorCode, int *message);

	const int m_masterRank;
	Graph* m_graph;
	int m_rank;
};