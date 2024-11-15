#ifndef GRAPH_H
#define GRAPH_H

#include "Node.h"
#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>

#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>
#include <map>

class Graph {
private:
    bool directed;
    std::unordered_map<std::string, Node*> nodes;

    // Mapa para almacenar los pesos originales de las aristas
    std::map<std::pair<std::string, std::string>, double> originalEdges;

public:
    Graph();
    Graph(bool directed);
    void addNode(const std::string& id, float x, float y);
    void addEdge(const std::string& src, const std::string& dest, double weight, bool isBidirectional);
    void addBidirectionalEdge(const std::string& src, const std::string& dest, double weight);
    void updateEdgeWeight(const std::string& src, const std::string& dest, double newWeight);

    // Método para restablecer los pesos a los valores originales
    void resetEdgeWeights();

    std::vector<std::string> getVertices();
    std::vector<std::tuple<std::string, double, bool>> getNeighbors(const std::string& vertex);

    Node* getNode(const std::string& id);
    bool isDirected();

    int getTotalCost(const std::vector<double>& wayCost, int costPerWeight);
    double getEdgeWeight(const std::string& src, const std::string& dest) const;

    bool isDirectNeighbor(const std::string& src, const std::string& dest);
    bool isDoubleWay(const std::string& src, const std::string& dest);


    int getTotalCost(vector<double> wayCost, int costPerWeigth);
    void clear();


    ~Graph();
};

#endif // GRAPH_H
