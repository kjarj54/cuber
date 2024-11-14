#ifndef FLOYDWARSHALL_H
#define FLOYDWARSHALL_H

#include "Graph.h"
#include <vector>
#include <string>
#include <limits>
#include <unordered_map>
#include "Incident.h"

class FloydWarshall {
public:
    FloydWarshall(Graph* graph);

    // Calcula los caminos m�s cortos entre todos los pares de nodos
    void calculateShortestPaths();

    // Actualiza las matrices de distancia y de nodo siguiente despu�s de un cambio en los pesos
    void updateMatrices(const std::vector<Incident>& incidents); // <-- Declaraci�n del m�todo updateMatrices aqu�
    void updateMatrices();
    // Obtiene el camino m�s corto entre dos nodos espec�ficos
    std::vector<std::string> getShortestPath(const std::string& start, const std::string& end, const vector<Incident>& incidents);

    // Calcula el costo total de transporte entre dos nodos, usando peso y costo de detenci�n
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
