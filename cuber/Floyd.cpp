#include "Floyd.h"
#include <iostream>
#include <limits>
#include <algorithm>

FloydWarshall::FloydWarshall(Graph* graph) : graph(graph) {
    initializeMatrices();
    calculateShortestPaths();
}

void FloydWarshall::initializeMatrices() {
    int n = graph->getVertices().size();
    distMatrix.resize(n, std::vector<double>(n, std::numeric_limits<double>::infinity()));
    nextNode.resize(n, std::vector<int>(n, -1));

    int index = 0;
    for (const auto& vertex : graph->getVertices()) {
        nodeIndex[vertex] = index;
        indexNode[index] = vertex;
        ++index;
    }

    for (const auto& vertex : graph->getVertices()) {
        int u = nodeIndex[vertex];
        distMatrix[u][u] = 0;

        for (const auto& neighbor : graph->getNeighbors(vertex)) {
            std::string neighborId;
            double weight;
            bool isBidirectional;
            std::tie(neighborId, weight, isBidirectional) = neighbor;

            int v = nodeIndex[neighborId];
            distMatrix[u][v] = weight;
            nextNode[u][v] = v;

            if (isBidirectional) {
                distMatrix[v][u] = weight;
                nextNode[v][u] = u;
            }
        }
    }
}

void FloydWarshall::calculateShortestPaths() {
    int n = distMatrix.size();
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (distMatrix[i][k] + distMatrix[k][j] < distMatrix[i][j]) {
                    distMatrix[i][j] = distMatrix[i][k] + distMatrix[k][j];
                    nextNode[i][j] = nextNode[i][k];
                }
            }
        }
    }
}

std::vector<std::string> FloydWarshall::getShortestPath(const std::string& start, const std::string& end) {
    int u = nodeIndex[start];
    int v = nodeIndex[end];

    if (nextNode[u][v] == -1) {
        return {}; // No hay camino
    }

    std::vector<std::string> path;
    while (u != v) {
        path.push_back(indexNode[u]);
        u = nextNode[u][v];
    }
    path.push_back(indexNode[v]);
    return path;
}

double FloydWarshall::calculateTransportCost(const std::string& start, const std::string& end, double costPerWeight, double costPerStop) {
    int u = nodeIndex[start];
    int v = nodeIndex[end];

    if (nextNode[u][v] == -1) {
        std::cerr << "No existe un camino entre los nodos " << start << " y " << end << std::endl;
        return std::numeric_limits<double>::infinity(); // Camino no disponible
    }

    double totalCost = 0.0;
    while (u != v) {
        int next = nextNode[u][v];
        double edgeWeight = distMatrix[u][next];
        totalCost += edgeWeight * costPerWeight; // Costo por peso
        totalCost += costPerStop; // Costo adicional por detenerse en el nodo
        u = next;
    }

    return totalCost;
}

