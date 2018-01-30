#pragma once

class Graph;

class Master
{
public:
	Master();
	void Run();
	void InitGraph();
	void FindAllPaths();
private:
	void ShowInitGraphText();
	void ShowMenu();
	void ClearScreen();
	Graph* m_graph;
};