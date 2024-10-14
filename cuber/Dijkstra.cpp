#include "Dijkstra.h"


Dijkstra::Dijkstra(Graph* graph) {
	this->graph = graph;
}

Dijkstra::Dijkstra() {
	this->graph = new Graph();
}

vector<string> Dijkstra::shortestPath(string& src, string& dest) {
	unordered_map<string, double> dist;
	unordered_map<string, string> prev;
	vector<string> vertices = this->graph->getVertices();
	for (int i = 0; i < vertices.size(); i++) {
		dist[vertices[i]] = numeric_limits<double>::infinity();
		prev[vertices[i]] = "";
	}
	dist[src] = 0;
	priority_queue<pair<double, string>, vector<pair<double, string>>, greater<pair<double, string>>> pq;
	pq.push(make_pair(0, src));
	while (!pq.empty()) {
		pair<double, string> top = pq.top();
		pq.pop();
		string u = top.second;
		vector<pair<string, double>> neighbors = this->graph->getNeighbors(u);
		for (int i = 0; i < neighbors.size(); i++) {
			string v = neighbors[i].first;
			double weight = neighbors[i].second;
			if (dist[u] + weight < dist[v]) {
				dist[v] = dist[u] + weight;
				prev[v] = u;
				pq.push(make_pair(dist[v], v));
			}
		}
	}
	vector<string> path;
	for (string at = dest; at != ""; at = prev[at]) {
		path.push_back(at);
	}
	reverse(path.begin(), path.end());
	return path;
}

Dijkstra::~Dijkstra() {
	delete this->graph;
}