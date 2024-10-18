#ifndef GRAPH_H
#define GRAPH_H

#include "Node.h"
#include <unordered_map>

class Graph {
private:
    bool directed;
    unordered_map<string, Node*> nodes;
public:
    Graph();
    Graph(bool directed);
    void addNode(string& id, float x, float y);
    void addEdge(string& src, string& dest, double weight);
    void addBidirectionalEdge(string& src, string& dest, double weight); // Nuevo método
    vector<string> getVertices();
    vector<pair<string, double>> getNeighbors(string& vertex);
    Node* getNode(string& id);
    bool isDirected();
    ~Graph();
};

#endif // GRAPH_H