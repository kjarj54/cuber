#ifndef GRAPH_H
#define GRAPH_H

#include "Node.h"
#include <unordered_map>
#include <vector>
#include <tuple>
#include <string>

class Graph {
private:
    bool directed;
    unordered_map<string, Node*> nodes;

public:
    Graph();
    Graph(bool directed);
    void addNode(const string& id, float x, float y);
    void addEdge(const string& src, const string& dest, double weight, bool isBidirectional);
    void addBidirectionalEdge(const string& src, const string& dest, double weight);
    void updateEdgeWeight(const std::string& src, const std::string& dest, double newWeight);


    vector<string> getVertices();

    // Asegúrate de que esta declaración coincida con la definición en Graph.cpp
    vector<tuple<string, double, bool>> getNeighbors(const string& vertex) ;

    Node* getNode(const string& id);
    bool isDirected();

    int getTotalCost(const vector<double>& wayCost, int costPerWeight);
    double getEdgeWeight(const std::string& src, const std::string& dest) const;

    bool isDirectNeighbor(const std::string& src, const std::string& dest);
    bool isDoubleWay(const std::string& src, const std::string& dest);
    int getTotalCost(vector<double> wayCost, int costPerWeigth);

    ~Graph();
};

#endif // GRAPH_H
