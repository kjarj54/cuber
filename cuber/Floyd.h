#ifndef FLOYDWARSHALL_H
#define FLOYDWARSHALL_H

#include "Graph.h"
#include <vector>
#include <string>
#include <limits>
#include <unordered_map>

class FloydWarshall {
public:
    FloydWarshall(Graph* graph);

    // Calcula los caminos más cortos entre todos los pares de nodos
    void calculateShortestPaths();

    // Obtiene el camino más corto entre dos nodos específicos
    std::vector<std::string> getShortestPath(const std::string& start, const std::string& end);

    // Calcula el costo total de transporte entre dos nodos, usando peso y costo de detención
    double calculateTransportCost(const std::string& start, const std::string& end, double costPerWeight, double costPerStop);

private:
    Graph* graph;
    std::vector<std::vector<double>> distMatrix;
    std::vector<std::vector<int>> nextNode;
    std::unordered_map<std::string, int> nodeIndex;
    std::unordered_map<int, std::string> indexNode;

    // Inicializa las matrices de distancia y de nodo siguiente
    void initializeMatrices();
};

#endif
