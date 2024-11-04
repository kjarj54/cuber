#include "Graph.h"

Graph::Graph() {
    this->directed = false;
}

Graph::Graph(bool directed) : directed(directed) {}

void Graph::addNode(string& id, float x, float y) {
    if (nodes.find(id) == nodes.end()) {
        nodes[id] = new Node(id, x, y);
    }
}

void Graph::addEdge(string& src, string& dest, double weight) {
    if (nodes.find(src) == nodes.end() || nodes.find(dest) == nodes.end()) {
        return;
    }
    nodes[src]->addNeighbor(dest, weight);
    if (!directed) {
        nodes[dest]->addNeighbor(src, weight);
    }
}

void Graph::addBidirectionalEdge(string& src, string& dest, double weight) {
    if (nodes.find(src) == nodes.end() || nodes.find(dest) == nodes.end()) {
        return;
    }
    nodes[src]->addNeighbor(dest, weight);
    nodes[dest]->addNeighbor(src, weight);
}

vector<string> Graph::getVertices() {
    vector<string> vertices;
    for (auto& pair : nodes) {
        vertices.push_back(pair.first);
    }
    return vertices;
}

vector<pair<string, double>> Graph::getNeighbors(string& vertex) {
    return nodes.at(vertex)->getNeighbors();
}

Node* Graph::getNode(string& id) {
    return nodes.at(id);
}

bool Graph::isDirected() {
    return directed;
}

// Verifica si 'dest' es vecino directo de 'src'
bool Graph::isDirectNeighbor(const std::string& src, const std::string& dest) {
    if (nodes.find(src) != nodes.end()) {
        for (const auto& neighbor : nodes[src]->getNeighbors()) {
            if (neighbor.first == dest) {
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

int Graph::getTotalCost(vector<double> wayCost, int costPerWeigth) {
    
    double total = 0;
    for (double c : wayCost) {

        total += c;
    }
    return total * costPerWeigth;
}

Graph::~Graph() {
    for (auto it = this->nodes.begin(); it != this->nodes.end(); it++) {
        delete it->second;
    }
}