#pragma once

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
private:
	void ShowError(int errorCode, char *errorMsg = nullptr);
	void ShowInitGraphText();
	void ShowMenu();
	void ClearScreen();
	Graph* m_graph;
	int m_rank;
};