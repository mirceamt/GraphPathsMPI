#pragma once

class Node;

class Graph
{
public:
	Graph();
	static Graph* GetInstance();
	void FindAllPaths(Node* start, Node* end);

};