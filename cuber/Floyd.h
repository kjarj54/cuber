#ifndef FLOYD_H
#define FLOYD_H
#include "Graph.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <limits>

class Floyd {
private:
	Graph* graph;
	unordered_map<string, unordered_map<string, double>> dist;
	unordered_map<string, unordered_map<string, string>> next;
	void initialize();
	void computePaths();

public:
	Floyd(Graph* graph);
	Floyd();
	vector<string> shortestPath(string& src, string& dest);
	~Floyd();
};


#endif // FLOYD_H