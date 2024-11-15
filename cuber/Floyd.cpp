#include "Floyd.h"
#include <iostream>
#include <limits>
#include <algorithm>

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

    updateMatrices();
}

void FloydWarshall::updateMatrices(const std::vector<Incident>& incidents) {
    int n = graph->getVertices().size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            distMatrix[i][j] = (i == j) ? 0 : std::numeric_limits<double>::infinity();
            nextNode[i][j] = -1;
        }
    }

    for (const auto& vertex : graph->getVertices()) {
        int u = nodeIndex[vertex];
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

    // Aplicar incidentes para bloquear rutas
    for (const auto& incident : incidents) {
        int u = nodeIndex[incident.fromPoint];
        int v = nodeIndex[incident.toPoint];

        if (incident.direction == "Ambas Direcciones" || incident.direction == "Direccion 1") {
            distMatrix[u][v] = std::numeric_limits<double>::infinity();
            nextNode[u][v] = -1;
        }
        if (incident.direction == "Ambas Direcciones" || incident.direction == "Direccion 2") {
            distMatrix[v][u] = std::numeric_limits<double>::infinity();
            nextNode[v][u] = -1;
        }
    }

    calculateShortestPaths(); // Recalcula los caminos más cortos
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

vector<std::string> FloydWarshall::getShortestPath(const std::string& start, const std::string& end, const vector<Incident>& incidents) {
    updateMatrices(incidents); // Recalcula la matriz con los incidentes
    updateMatrices();
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


void FloydWarshall::updateMatrices() {
    int n = graph->getVertices().size();
    std::cout << "Inicializando matrices de distancias y nodos...\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            distMatrix[i][j] = (i == j) ? 0 : std::numeric_limits<double>::infinity();
            nextNode[i][j] = -1;
        }
    }

    std::cout << "Rellenando matrices con los pesos actualizados del grafo...\n";
    for (const auto& vertex : graph->getVertices()) {
        int u = nodeIndex[vertex];
        for (const auto& neighbor : graph->getNeighbors(vertex)) {
            std::string neighborId;
            double weight;
            bool isBidirectional;
            std::tie(neighborId, weight, isBidirectional) = neighbor;

            int v = nodeIndex[neighborId];

            // Obtén siempre el peso actualizado desde el grafo
            double updatedWeight = graph->getEdgeWeight(vertex, neighborId);


            distMatrix[u][v] = updatedWeight;
            nextNode[u][v] = v;

            if (isBidirectional) {
                distMatrix[v][u] = updatedWeight;
                nextNode[v][u] = u;
            }

            std::cout << "Arista " << vertex << " -> " << neighborId
                << " con peso actualizado " << updatedWeight
                << (isBidirectional ? " (bidireccional)" : "") << "\n";
        }
    }

    calculateShortestPaths(); // Recalcula los caminos más cortos con la matriz actualizada
}