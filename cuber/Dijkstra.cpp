#include "Dijkstra.h"
#include <iostream>

Dijkstra::Dijkstra(Graph* graph) : graph(graph) {}

std::vector<std::string> Dijkstra::shortestPath(const std::string& src, const std::string& dest) {
    // Mapa de distancias y predecesores
    std::unordered_map<std::string, double> distances;
    std::unordered_map<std::string, std::string> predecessors;

    for (const auto& node : graph->getVertices()) {
        distances[node] = std::numeric_limits<double>::infinity();
    }
    distances[src] = 0;

    using NodeDistPair = std::pair<double, std::string>;
    std::priority_queue<NodeDistPair, std::vector<NodeDistPair>, std::greater<NodeDistPair>> pq;
    pq.push({ 0, src });

    while (!pq.empty()) {
        double currentDist = pq.top().first;
        std::string currentNode = pq.top().second;
        pq.pop();

        if (currentNode == dest) break;

        for (const auto& neighborTuple : graph->getNeighbors(currentNode)) {
            std::string neighbor;
            double weight;
            bool bidirectional;

            std::tie(neighbor, weight, bidirectional) = neighborTuple;
            double newDist = currentDist + weight;

            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                predecessors[neighbor] = currentNode;
                pq.push({ newDist, neighbor });
            }
        }
    }

    std::vector<std::string> path;
    for (std::string at = dest; at != src; at = predecessors[at]) {
        path.push_back(at);
        if (predecessors.find(at) == predecessors.end()) {
            std::cout << "No se encontró camino entre " << src << " y " << dest << std::endl;
            return {};
        }
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());

    return path;
}
