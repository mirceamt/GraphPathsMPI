#pragma once
#include <string>

class Graph;

class Master
{
public:
	Master(int rank);
	void Init();
	void Run();
	void CleanUp();
	static void Log(std::string s);
	static int Rank();

private:
	void InitGraph();
	void FindAllPaths();
	void FindShortestPath();
	void FindLongestPath();
	void BroadcastOption(int option);

	void ComputeNodeIntervals(int *msg);

	void ShowInitGraphText(char *s);
	void ShowMenu();
	void ClearScreen();
	Graph* m_graph;
	static int m_rank;
};