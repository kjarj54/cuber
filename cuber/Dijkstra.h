#include <string>
#include <vector>
#include "Graph.h"
#include "Incident.h"

class Dijkstra {
public:
    Dijkstra(Graph* graph);
    std::vector<std::string> shortestPath(const std::string& startNodeId, const std::string& endNodeId, const std::vector<Incident>& incidents);

private:
    Graph* graph;
};
