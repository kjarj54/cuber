#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Graph.h"
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <limits>

class Dijkstra {
private:
	Graph* graph;
public:
	Dijkstra(Graph* graph);
	Dijkstra();
	vector<string> shortestPath(string& src, string& dest);
	~Dijkstra();
};

#endif
