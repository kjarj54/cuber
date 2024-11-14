#include <iostream>
#include <string>
#include <vector>
#include <tuple>

using namespace std;

class Node {
private:
    string id;
    float x, y;
    vector<tuple<string, double, bool>> neighbors;  // Cada vecino es una tupla con (ID, peso, bidireccionalidad)

public:
    Node();
    Node(const string& id, float x, float y);

    string getId() const;
    float getX() const;
    float getY() const;

    void addNeighbor(const string& neighborId, double weight, bool isBidirectional);
    std::vector<std::tuple<std::string, double, bool>>& getNeighbors();


    ~Node();
};
