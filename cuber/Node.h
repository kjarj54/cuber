#ifndef NODE_H
#define	NODE_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Node {

private:
	string id;
	float x, y;
	vector<pair<string, double>> neighbors;
public:
	Node();
	Node(string& id, float x, float y);
	string getId();
	float getX();
	float getY();
	void addNeighbor(string& neighborId, double weight);
	vector<pair<string, double>> getNeighbors();
	~Node();

};

#endif // NODE_H
