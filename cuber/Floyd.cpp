#include "Dijkstra.h"
#include <queue>
#include <functional>
#include <limits>
#include <tuple>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>

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

    for (const auto& vertex : vertices) {
        dist[vertex] = numeric_limits<double>::infinity();
        prev[vertex] = "";
    }

    dist[src] = 0;
    priority_queue<pair<double, string>, vector<pair<double, string>>, greater<pair<double, string>>> pq;
    pq.push(make_pair(0, src));

    while (!pq.empty()) {
        pair<double, string> top = pq.top();
        pq.pop();
        string u = top.second;

        vector<tuple<string, double, bool>> neighbors = this->graph->getNeighbors(u);

        for (const auto& neighbor : neighbors) {
            string v = std::get<0>(neighbor);
            double weight = std::get<1>(neighbor);
            bool isBidirectional = std::get<2>(neighbor);

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
