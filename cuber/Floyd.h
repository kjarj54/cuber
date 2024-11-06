#ifndef FLOYDWARSHALL_H
#define FLOYDWARSHALL_H

#include "Graph.h"
#include <vector>
#include <string>
#include <limits>

class FloydWarshall {
public:
    FloydWarshall(Graph* graph);
    void calculateShortestPaths();
    std::vector<std::string> getShortestPath(const std::string& start, const std::string& end);

private:
    Graph* graph;
    std::vector<std::vector<double>> distMatrix;
    std::vector<std::vector<int>> nextNode;
    std::unordered_map<std::string, int> nodeIndex;
    std::unordered_map<int, std::string> indexNode;

    void initializeMatrices();
};

#endif
