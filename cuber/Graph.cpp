#include "Graph.h"

Graph::Graph() : directed(false) {}

Graph::Graph(bool directed) : directed(directed) {}

void Graph::addNode(const string& id, float x, float y) {
    if (nodes.find(id) == nodes.end()) {
        nodes[id] = new Node(id, x, y);
    }
}

void Graph::addEdge(const string& src, const string& dest, double weight, bool isBidirectional) {
    if (nodes.find(src) == nodes.end() || nodes.find(dest) == nodes.end()) {
        return;
    }
    nodes[src]->addNeighbor(dest, weight, isBidirectional);
    if (isBidirectional || !directed) {
        nodes[dest]->addNeighbor(src, weight, isBidirectional);
    }
}


void Graph::addBidirectionalEdge(const string& src, const string& dest, double weight) {
    if (nodes.find(src) == nodes.end() || nodes.find(dest) == nodes.end()) {
        return;
    }
    nodes[src]->addNeighbor(dest, weight, true);
    nodes[dest]->addNeighbor(src, weight, true);
}

vector<string> Graph::getVertices() {
    vector<string> vertices;
    for (const auto& pair : nodes) {
        vertices.push_back(pair.first);
    }
    return vertices;
}

// Definición de getNeighbors en Graph.cpp
vector<tuple<string, double, bool>> Graph::getNeighbors(const string& vertex) {
    vector<tuple<string, double, bool>> neighbors;

    if (nodes.find(vertex) != nodes.end()) {
        for (const auto& neighbor : nodes[vertex]->getNeighbors()) {
            // Usa std::get para obtener cada elemento de la tupla
            string neighborId = std::get<0>(neighbor);
            double weight = std::get<1>(neighbor);
            bool isBidirectional = std::get<2>(neighbor);

            neighbors.push_back(make_tuple(neighborId, weight, isBidirectional));
        }
    }

    return neighbors;
}



Node* Graph::getNode(const string& id) {
    if (nodes.find(id) != nodes.end()) {
        return nodes.at(id);
    }
    return nullptr;
}

bool Graph::isDirected() {
    return directed;
}


// Verifica si 'dest' es vecino directo de 'src'
bool Graph::isDirectNeighbor(const std::string& src, const std::string& dest) {
    if (nodes.find(src) != nodes.end()) {
        for (const auto& neighbor : nodes[src]->getNeighbors()) {
            if (get<0>(neighbor) == dest) {
                return true;
            }
        }
    }
    return false;
}

// Verifica si hay doble sentido entre 'src' y 'dest'
bool Graph::isDoubleWay(const std::string& src, const std::string& dest) {
    // Comprobar si ambos son vecinos directos entre sí
    if (isDirectNeighbor(src, dest) && isDirectNeighbor(dest, src)) {
        return true;
    }
    return false;
}

int Graph::getTotalCost(const vector<double>& wayCost, int costPerWeight) {
    double total = 0;
    for (double c : wayCost) {
        total += c;
    }
    return static_cast<int>(total * costPerWeight);
}

Graph::~Graph() {
    for (auto it = this->nodes.begin(); it != this->nodes.end(); ++it) {
        delete it->second;
    }
}
