#include "Node.h"

Node::Node() : id(""), x(0), y(0) {}

Node::Node(const string& id, float x, float y) : id(id), x(x), y(y) {}

string Node::getId() const {
    return id;
}

float Node::getX() const {
    return x;
}

float Node::getY() const {
    return y;
}

void Node::addNeighbor(const string& neighborId, double weight, bool isBidirectional) {
    neighbors.emplace_back(neighborId, weight, isBidirectional);
}

vector<tuple<string, double, bool>> Node::getNeighbors() const {
    return neighbors;
}

Node::~Node() {
    // Destructor
}
