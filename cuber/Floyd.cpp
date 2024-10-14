#include "Floyd.h"

Floyd::Floyd(Graph* graph) {
	this->graph = graph;
	this->initialize();
	this->computePaths();
}

Floyd::Floyd() {
	this->graph = new Graph();
	this->initialize();
	this->computePaths();
}

void Floyd::initialize() {
	vector<string> vertices = this->graph->getVertices();
	for (string& src : vertices) {
		this->dist[src] = unordered_map<string, double>();
		this->next[src] = unordered_map<string, string>();
		for (string& dest : vertices) {
			this->dist[src][dest] = numeric_limits<double>::infinity();
			this->next[src][dest] = "";
		}
		this->dist[src][src] = 0;
		vector<pair<string, double>> neighbors = this->graph->getNeighbors(src);
		for (pair<string, double>& neighbor : neighbors) {
			this->dist[src][neighbor.first] = neighbor.second;
			this->next[src][neighbor.first] = neighbor.first;
		}
	}
}

void Floyd::computePaths() {
	vector<string> vertices = this->graph->getVertices();
	for (string& k : vertices) {
		for (string& i : vertices) {
			for (string& j : vertices) {
				if (this->dist[i][j] > this->dist[i][k] + this->dist[k][j]) {
					this->dist[i][j] = this->dist[i][k] + this->dist[k][j];
					this->next[i][j] = this->next[i][k];
				}
			}
		}
	}
}

vector<string> Floyd::shortestPath(string& src, string& dest) {
	vector<string> path;
	if (this->next[src][dest] == "") {
		return path;
	}
	path.push_back(src);
	for (string at = src; at != dest; at = this->next[at][dest]) {
		if (at == "") {
			path.clear();
			return path;
		}
		path.push_back(this->next[at][dest]);
	}
	return path;
}


Floyd::~Floyd() {
	delete this->graph;
}
