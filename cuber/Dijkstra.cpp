#include "Dijkstra.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>

Dijkstra::Dijkstra(Graph* graph) : graph(graph) {}

std::vector<std::string> Dijkstra::shortestPath(const std::string& startNodeId, const std::string& endNodeId, const std::vector<Incident>& incidents) {
    std::unordered_map<std::string, int> distances;  // Cambiado a int para contar saltos
    std::unordered_map<std::string, std::string> previous;
    std::queue<std::string> q;  // Usamos una cola simple para BFS

    // Inicializar todas las distancias como "infinito" (máximo de saltos)
    for (const auto& vertex : graph->getVertices()) {
        distances[vertex] = std::numeric_limits<int>::max();
    }

    distances[startNodeId] = 0;
    q.push(startNodeId);

    while (!q.empty()) {
        std::string currentNode = q.front();
        q.pop();

        if (currentNode == endNodeId) break;

        for (const auto& neighbor : graph->getNeighbors(currentNode)) {
            std::string neighborId;
            bool isBidirectional;
            std::tie(neighborId, std::ignore, isBidirectional) = neighbor;  // Ignoramos el peso

            // Verificar si hay un incidente que afecta esta conexión
            bool skipConnection = false;
            for (const auto& incident : incidents) {
                // Verificar Dirección 1, Dirección 2, y Ambas Direcciones
                if ((incident.fromPoint == currentNode && incident.toPoint == neighborId && incident.direction == "Direccion 1") ||
                    (incident.fromPoint == neighborId && incident.toPoint == currentNode && incident.direction == "Direccion 2") ||
                    ((incident.fromPoint == currentNode && incident.toPoint == neighborId) ||
                        (incident.fromPoint == neighborId && incident.toPoint == currentNode)) &&
                    incident.direction == "Ambas Direcciones") {
                    skipConnection = true;
                    break;
                }
            }

            if (skipConnection) continue;

            // Sumamos 1 para representar un salto, en lugar de usar el peso de la arista
            int newDist = distances[currentNode] + 1;
            if (newDist < distances[neighborId]) {
                distances[neighborId] = newDist;
                previous[neighborId] = currentNode;
                q.push(neighborId);  // Añadir vecino a la cola
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
