#include "Node.h"

Node::Node(){
    id = "";
	x = 0;
	y = 0;

}

Node::Node( string& id, float x, float y) : id(id), x(x), y(y) {}

string Node::getId()  {
    return id;
}

float Node::getX()  {
    return x;
}

float Node::getY()  {
    return y;
}

void Node::addNeighbor( string& neighborId, double weight) {
    neighbors.emplace_back(neighborId, weight);
}

vector<pair<string, double>> Node::getNeighbors()  {
    return neighbors;
}

Node::~Node() {
	neighbors.clear();
}