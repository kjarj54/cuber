#include "Graph.h"

Graph::Graph() : directed(false) {}

Graph::Graph(bool directed) : directed(directed) {}

void Graph::addNode(const string& id, float x, float y) {
    if (nodes.find(id) == nodes.end()) {
        nodes[id] = new Node(id, x, y);
    }
}


void Graph::addEdge(const std::string& src, const std::string& dest, double weight, bool isBidirectional) {
    if (nodes.find(src) == nodes.end() || nodes.find(dest) == nodes.end()) {
        return;
    }

    // Agrega la arista al grafo
    nodes[src]->addNeighbor(dest, weight, isBidirectional);
    if (isBidirectional || !directed) {
        nodes[dest]->addNeighbor(src, weight, isBidirectional);
    }

    // Almacena el peso original si aún no está en el mapa
    if (originalEdges.find({ src, dest }) == originalEdges.end()) {
        originalEdges[{src, dest}] = weight;
    }
    if ((isBidirectional || !directed) && originalEdges.find({ dest, src }) == originalEdges.end()) {
        originalEdges[{dest, src}] = weight;
    }
}


void Graph::resetEdgeWeights() {
    for (const auto& edge : originalEdges) {
        const auto& src = edge.first.first;
        const auto& dest = edge.first.second;
        double originalWeight = edge.second;

        // Actualiza el peso de la arista al valor original
        updateEdgeWeight(src, dest, originalWeight);
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

double Graph::getEdgeWeight(const std::string& src, const std::string& dest) const {
    if (nodes.find(src) != nodes.end()) {
        for (const auto& neighbor : nodes.at(src)->getNeighbors()) {
            if (std::get<0>(neighbor) == dest) {
                return std::get<1>(neighbor); // Devuelve el peso de la arista
            }
        }
    }
    // Si no se encuentra la arista, devuelve -1 para indicar que no existe
    return -1.0;
}

void Graph::updateEdgeWeight(const std::string& src, const std::string& dest, double newWeight) {
    if (nodes.find(src) != nodes.end() && nodes.find(dest) != nodes.end()) {
        for (auto& neighbor : nodes[src]->getNeighbors()) {
            if (std::get<0>(neighbor) == dest) {
                std::get<1>(neighbor) = newWeight;  // Actualiza el peso
                std::cout << "Peso actualizado para " << src << " -> " << dest << ": " << newWeight << std::endl;
                break;
            }
        }
        if (!directed || isDoubleWay(src, dest)) {
            for (auto& neighbor : nodes[dest]->getNeighbors()) {
                if (std::get<0>(neighbor) == src) {
                    std::get<1>(neighbor) = newWeight;  // Actualiza el peso en la dirección opuesta
                    std::cout << "Peso actualizado para " << dest << " -> " << src << ": " << newWeight << std::endl;
                    break;
                }
            }
        }
    }
    else {
        std::cerr << "Error: Uno de los nodos no existe en el grafo." << std::endl;
    }
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
