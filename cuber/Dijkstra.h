#include <string>
#include <vector>
#include "Graph.h"

class Dijkstra {
public:
    Dijkstra(Graph* graph);
    std::vector<std::string> shortestPath(const std::string& startNodeId, const std::string& endNodeId);

private:
    Graph* graph;
};
