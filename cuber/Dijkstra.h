#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "Graph.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <limits>
#include <queue>

class Dijkstra {
private:
    Graph* graph;

public:
    Dijkstra(Graph* graph);
    std::vector<std::string> shortestPath(const std::string& src, const std::string& dest);
};

#endif // DIJKSTRA_H
