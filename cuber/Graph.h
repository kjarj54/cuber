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
    vector<string> getVertices();

    // Aseg�rate de que esta declaraci�n coincida con la definici�n en Graph.cpp
    vector<tuple<string, double, bool>> getNeighbors(const string& vertex) ;

    Node* getNode(const string& id);
    bool isDirected();
    int getTotalCost(const vector<double>& wayCost, int costPerWeight);
    ~Graph();
};

#endif // GRAPH_H
