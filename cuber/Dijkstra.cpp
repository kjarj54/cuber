#include "Dijkstra.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>

Dijkstra::Dijkstra(Graph* graph) : graph(graph) {}

std::vector<std::string> Dijkstra::shortestPath(const std::string& startNodeId, const std::string& endNodeId, const std::vector<Incident>& incidents) {
    std::unordered_map<std::string, double> distances;
    std::unordered_map<std::string, std::string> previous;
    std::priority_queue<std::pair<double, std::string>, std::vector<std::pair<double, std::string>>, std::greater<>> pq;

    // Inicializar todas las distancias como "infinito"
    for (const auto& vertex : graph->getVertices()) {
        distances[vertex] = std::numeric_limits<double>::infinity();
    }

    distances[startNodeId] = 0.0;
    pq.push({ 0.0, startNodeId });

    while (!pq.empty()) {
        auto top = pq.top();
        double currentDist = top.first;
        std::string currentNode = top.second;
        pq.pop();

        if (currentNode == endNodeId) break;

        if (currentDist > distances[currentNode]) continue;

        for (const auto& neighbor : graph->getNeighbors(currentNode)) {
            std::string neighborId;
            double weight;
            bool isBidirectional;
            std::tie(neighborId, weight, isBidirectional) = neighbor;

            bool skipConnection = false;
            for (const auto& incident : incidents) {
                if ((incident.fromPoint == currentNode && incident.toPoint == neighborId && incident.direction == "Direccion 1") ||
                    (incident.fromPoint == neighborId && incident.toPoint == currentNode && incident.direction == "Direccion 2") ||
                    (incident.fromPoint == currentNode && incident.toPoint == neighborId && incident.direction == "Ambas Direcciones")) {
                    skipConnection = true;
                    break;
                }
            }

            if (skipConnection) continue;

            double newDist = currentDist + weight;
            if (newDist < distances[neighborId]) {
                distances[neighborId] = newDist;
                previous[neighborId] = currentNode;
                pq.push({ newDist, neighborId });
            }
        }
    }

    // Reconstruir el camino desde `endNodeId` hacia `startNodeId`
    std::vector<std::string> path;
    for (std::string at = endNodeId; at != ""; at = previous[at]) {
        path.push_back(at);
        if (at == startNodeId) break;
    }
    std::reverse(path.begin(), path.end());

    if (path.front() != startNodeId) {
        return {}; // Devuelve un camino vacío si no se encuentra el camino entre los nodos
    }
    return path;
}
