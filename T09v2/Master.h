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
	void InitGraph();
	void FindAllPaths();
	static void Log(std::string s);

private:
	void BroadcastOption(int option);
	void ShowInitGraphText(char *s);
	void ShowMenu();
	void ClearScreen();
	Graph* m_graph;
	int m_rank;
};